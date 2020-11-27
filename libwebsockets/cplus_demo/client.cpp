#include <string>
#include <iostream>
#include <libwebsockets.h>
#include "orcv_lws.hpp"

using std::cout;
using std::endl;

#define MAX_COUNT 10000
int msg_count = 0;

const char *text = "1AAAAAAAA2BBBBBBBBB3CCCCCCCCC4DDDDDDDDDD5EEEEEEEEE6FFFFFFFFF7GGGGGGGGGG8HHHHHHHHHH9IIIIIIIIII10JJJJJJJJ11KKKKKKKK12LLLLLLLL13MMMMMMMM14NNNNNNNN15OOOOOOOO16PPPPPPPP17QQQQQQQQ18RRRRRRRR19SSSSSSSS20TTTTTT";

int callback_client(struct lws *wsi,
                    enum lws_callback_reasons reason,
                    void *user, void *in, size_t len)
{
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED: { // connect success
            cout << "[lws] connection established" << endl;
            break;
        }
        case LWS_CALLBACK_CLIENT_RECEIVE: {
            //lwsl_err( "%s\n", (char *) in );
            printf("%s\n", (char *)in); fflush(stdout);
            break;
        }
        case LWS_CALLBACK_CLIENT_WRITEABLE: {// 当此客户端可以发送数据时的回调
            if (msg_count < MAX_COUNT) {
                size_t len;
                char buf[LWS_PRE + 1024] = "";
                // lws_write()前面LWS_PRE个字节必须留给LWS
                memset(buf, 0, LWS_PRE + 1024);
                char *msg = buf+LWS_PRE;
                len = sprintf(msg, "%s", text);
                lws_write(wsi, (unsigned char*)msg, len, LWS_WRITE_TEXT);

                lwsl_err("client send msg_no=%d msg_len=%ld\n", msg_count+1, len);
                //cout << msg << endl;
                msg_count++;
            }
            break;
        }
        default:
            break;
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
        .callback = callback_client,    // callback function name
        .per_session_data_size = LWS_PRE + 1024,     // ?
        .rx_buffer_size = (1024*512)    // ?
    },
    {
        NULL, NULL, 0                   // End of list, must have add this struct array element
    }
};

int main(int argc, char *argv[])
{
    int rc = -1;

    Orcvlws orcvlws(9090, "127.0.0.1", protocols);
    rc = orcvlws.CreateLocalContext();
    if(rc) {
        return rc;
    }

    rc = orcvlws.ConnService();
    if(rc) {
        return rc;
    }
    orcvlws.SetSignal();
    // in while(true)
    orcvlws.WaitService();

    rc = 0;
    return rc;
}
