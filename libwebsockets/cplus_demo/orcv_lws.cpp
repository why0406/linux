#include <iostream>
#include <string>
#include <signal.h>
#include "orcv_lws.hpp"

using std::cout;
using std::endl;

static volatile bool exit_sig = false;

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

/*
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
            // receive "complete packets" from the client return rem = 0
            // rem means remaining unaccepted data bytes
            size_t rem = lws_remaining_packet_payload(wsi);
            if(rem == 0) {
                cout << "[lws] receive a compelete packets(" << len << "):[" << in << "]" << endl;
            } else {
                cout << "[lws] receive a uncompelete packets(" << len << "):[" << in << "]" << endl;
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
*/

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

    conn_wsi = lws_client_connect_via_info(&conn_info); // TODO : 是否需要释放
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
    cout << "---" << context_info.protocols[1].name << "---" <<context_info.protocols[1].callback << endl;
}

//==================================
// create local websocket as service
//==================================
Orcvlws::Orcvlws(uint32_t port, struct lws_protocols protocols[])
{
    this->port = port;
    this->addr = NULL;
    this->context = NULL;
    // local connext parameter
    memset((void *)&context_info, 0, sizeof(context_info));
    context_info.port = port;
    context_info.uid = -1;
    context_info.protocols = protocols; // set your protocols
    cout << "---" << context_info.protocols[1].name << "---" <<context_info.protocols[1].callback << endl;
}

Orcvlws::~Orcvlws()
{
    if(context) {
        lws_context_destroy(context);
        lwsl_notice( "lws_context_destroy successed\n");
    }
}
