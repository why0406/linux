#include <libwebsockets.h>
#include <string>
#include <iostream>
#include "orcv_lws.hpp"

using std::cout;
using std::endl;

int callback_server(struct lws *wsi,
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
        // 3. size_t len 是in的长度
        //==============================================================
        case LWS_CALLBACK_RECEIVE: {
            int fd = lws_get_socket_fd(wsi);
            // receive "complete packets" from the client return rem = 0
            // rem means remaining unaccepted data bytes
            size_t rem = lws_remaining_packet_payload(wsi);
            std::string instr((const char *)in, len);
            if(rem == 0) {
                cout << "[lws] receive a compelete packets(" << len << "):[" << instr << "]" << endl;
            } else {
                cout << "[lws] receive a uncompelete packets(" << len << "):[" << instr << "]" << endl;
            }
			// write back to client
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

struct lws_protocols protocols[] = { 
    /* first protocol must always be HTTP handler */
    {   
        .name = "http-only",
        .callback = callback_http,
        .per_session_data_size = 0 
    },  
    {   
        .name = "conn-protocol",        // protocol name - client and server should assign same protocol
        .callback = callback_server,    // callback function name
        .per_session_data_size = 0,     // ?
        .rx_buffer_size = (1024*512)    // ?
    },  
    {   
        NULL, NULL, 0                   // End of list, must have add this struct array element
    }   
};


int main(int argc, char *argv[])
{
    int rc = -1;
    // init a server
    Orcvlws orcvlws(9090, protocols);
    // create loacl context
    rc = orcvlws.CreateLocalContext();
    if(rc) {
        return rc;
    }
    orcvlws.SetSignal();
    // in while(true)
    orcvlws.WaitClient();

    rc = 0;
    return rc;
}
