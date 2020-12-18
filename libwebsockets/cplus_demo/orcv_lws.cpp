#include <iostream>
#include <string>
#include <signal.h>
#include "orcv_lws.hpp"

using std::cout;
using std::endl;

static volatile bool exit_sig = false;

//===============================================================================
// note:
//       接收缓冲区大小设置为LWS_RX_BUFFER_SIZE,单条msg理论最大值不会超过此值
//       如果单条消息等于LWS_MAX_MSG_SIZE,此消息可能是不完整的,应该被忽略
//       应该和客户端约定发送消息最大值为LWS_MAX_MSG_SIZE-1
// return:
//       0 -- alredy get complete
//       1 -- not get complete msg,
//            need to break and call next callback() function to get remaining msg
//================================================================================
int get_complete_msg(struct lws *wsi, CompleteMsg *p_completeMsg, void *in, size_t len)
{
    // receive "complete packets" from the client return rem = 0
    // rem means remaining unaccepted data bytes
    size_t rem = lws_remaining_packet_payload(wsi);
    // current fragment is last one in this packet return 1
    int isFinalFragment = lws_is_final_fragment(wsi);
    int isFirstFragment = lws_is_first_fragment(wsi);

    if((rem == 0) && isFirstFragment) {
        // current msg is compelete
        p_completeMsg->p_data = (char *)in;
        p_completeMsg->len = len;
        return 0;
    } else {
        // current msg is not complete

        // 每一条新的消息,len=0
        if((rem != 0) && isFirstFragment) {
            p_completeMsg->len = 0;
        }

        if(p_completeMsg->len + len >= LWS_MAX_MSG_SIZE) {
            cout << "[callback] ERROR : user msg data is out of memory!" << endl;
            p_completeMsg->len = LWS_MAX_MSG_SIZE;
            goto out;
        }

        p_completeMsg->p_data = p_completeMsg->data;
        memcpy(p_completeMsg->p_data + p_completeMsg->len, (char *)in, len);

        p_completeMsg->len += len;
out:
        if((rem == 0) && isFinalFragment) {
            // last fragment of uncomplete msg
            return 0;
        } else {
            return 1;
        }
    }
}

void sighdl(int sig)
{
    lwsl_notice( "signal [%d] traped\n", sig );
    exit_sig = true;
}

int callback_http(struct lws *wsi,
              enum lws_callback_reasons reason,
              void *user, void *in, size_t len)
{
    return 0;
}

#if 0
int callback_demo(struct lws *wsi,
                 enum lws_callback_reasons reason,
                 void *user, void *in, size_t len)
{
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED: { // connect success
            cout << "[lws] connection established" << endl;
            break;
        }

        //=============================================================
        // LWS_CALLBACK_RECEIVE means receive something
        // 1. 首先是要判断收到的消息是不是完整的包,
        //    不是的话要继续接收知道收到完整的消息,然后才能处理消息内容
        // 2. void *in 存储的是本次接收的消息,lws已经为其分配了内存
        //    但是in中只有前len个字节是client send的,剩余的是lws的其他信息
        //    因此，要对in 进行处理
        // 3. size_t len 是in的长度
        //==============================================================
        case LWS_CALLBACK_RECEIVE: {
            int fd = lws_get_socket_fd(wsi);
            int isFinalFragment = lws_is_final_fragment(wsi);
            // receive "complete packets" from the client return rem = 0
            // rem means remaining unaccepted data bytes
            size_t rem = lws_remaining_packet_payload(wsi);
            if(rem == 0) {
                cout << "[lws] receive a compelete packets(" << len << "):[" << isFinalFragment << "]" << in  << endl;
            } else {
                cout << "[lws] receive a uncompelete packets(" << len << "):[" << isFinalFragment << "]" << in  << endl;
            }
            // write back
            char write_buf[64] = "";
            size_t write_len;
            write_len = snprintf(write_buf, sizeof(write_buf), "[service] : receive from client > %s", (char *)in);
            if(write_len < 0) {
                cout << "[lws] snprintf error!" << endl;
                break;
            }

            int rc = lws_write(wsi, (unsigned char *)write_buf, write_len, LWS_WRITE_TEXT);
            if (rc == -1) {
                // TODO: need handle connection closed by somebody
            } else if ((size_t)rc != write_len) {
                // TODO: need handle not send complete
            }
            break;
        }

    }
        return 0;
}

static struct lws_protocols protocols_demo[] = { 
//first protocol must always be HTTP handler
    {   
        .name = "http-only",
            .callback = callback_http,
            .per_session_data_size = 0 
    },  
    {   
        .name = "demo-protocol",        // protocol name - client and server should assign same protocol
        .callback = callback_demo,      // callback function name
        .per_session_data_size = 0,     // ?
        .rx_buffer_size = (1024*512)    // ?
    },  
    {   
        NULL, NULL, 0                   // End of list, must have add this struct array element
    }   
};
#endif

void Orcvlws::SetSignal()
{
    signal(SIGINT, sighdl);
    signal(SIGTERM, sighdl);
}

void Orcvlws::WaitService()
{
    while(true) {
        if(exit_sig)
            break;
        lws_service(context, 50);

        /**
        * 下面的调用的意义是：当连接可以接受新数据时，触发一次WRITEABLE事件回调
        * 当连接正在后台发送数据时，它不能接受新的数据写入请求，所有WRITEABLE事件回调不会执行
        **/
        lws_callback_on_writable(conn_wsi);
    }
}

void Orcvlws::WaitClient()
{
    while(true) {
        if(exit_sig)
            break;
        lws_service(context, 50);
        // lws_service will process all waiting events with their
        // callback functions and then wait 50 ms.
        // (this is a single threaded webserver and this will keep our server
        // from generating load while there are not requests to process)
    }
}

//=============================================
// client call this fun to set server parameter 
// and Establish connection with server
//============================================
int Orcvlws::ConnService()
{
    int rc;
    char addr_port[256] = "";

    rc = snprintf(addr_port, sizeof(addr_port), "%s:%u", addr, port & 65535 );
    if(rc < 0) {
        lwsl_err("Failed to snprintf ip:port string\n");
        return -1;
    }

    memset((void *)&conn_info, 0, sizeof(conn_info));
    // All the following parameters must be assigned and cannot be missing anyone,
    // Otherwise,Segmentation fault will be reported
    conn_info.context = this->context;
    conn_info.address = addr;
    conn_info.port = port;
    conn_info.path = "./";
    conn_info.ssl_connection = 0; // ssl
    conn_info.host = addr_port;
    conn_info.origin = addr_port;
    conn_info.protocol = context_info.protocols[1].name;

    conn_wsi = lws_client_connect_via_info(&conn_info);
    if(conn_wsi == NULL) {
        lwsl_err("Failed to create lws_client_connect_via_info\n");
        return -1;
    }
    return 0;
}

//==================================
// create local websocket context
//==================================
int Orcvlws::CreateLocalContext()
{
    int rc;
    /* set lws log
    int logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_DEBUG;
    lws_set_log_level(logs, NULL);
    */

    this->context = lws_create_context(&context_info);
    if (this->context == NULL) {
        rc = -1;
        lwsl_err( "lws_create_context failed\n");
    } else {
        rc = 0;
        lwsl_notice( "lws_create_context successed\n");
    }
    return rc;
}

//==================================
// create local websocket as client
//==================================
Orcvlws::Orcvlws(uint32_t port, const char *addr, struct lws_protocols protocols[])
{
    this->port = port;
    this->addr = addr;
    this->context = NULL;
    // local connext parameter
    memset((void *)&context_info, 0, sizeof(context_info));
    context_info.port = CONTEXT_PORT_NO_LISTEN;
    context_info.iface = NULL;
    context_info.uid = -1;
    context_info.protocols = protocols;
}

//==================================
// create local websocket as service
//==================================
Orcvlws::Orcvlws(uint32_t port, struct lws_protocols protocols[])
{
    this->port = port; // not use
    this->addr = NULL;
    this->context = NULL;
    // local connext parameter
    memset((void *)&context_info, 0, sizeof(context_info));
    context_info.port = port;
    context_info.uid = -1;
    context_info.protocols = protocols; // set your protocols
}

Orcvlws::~Orcvlws()
{
    if(context) {
        lws_context_destroy(context);
        lwsl_notice( "lws_context_destroy successed\n");
    }
}
