#ifndef _ORCVLWS_
#define _ORCVLWS_
#include <libwebsockets.h>

#define LWS_USER_SIZE      (16*1024)
#define LWS_MAX_MSG_SIZE   (8*1024)
#define LWS_RX_BUFFER_SIZE (512*1024)

typedef struct _compelteMsg {
    size_t len;
    char *p_data;
    /*一次就接收到完整消息时p_data直接指向in,否则指向data[]并把in memcpy到data[],
    这样避免一次就接收到完整消息时也向data[]进行memcpy*/
    char data[LWS_MAX_MSG_SIZE];
} CompelteMsg;

int
get_complete_msg(struct lws *wsi, CompelteMsg *p_compelteMsg, void *in, size_t len);

int
callback_http(struct lws *wsi, enum lws_callback_reasons reason,
                  void *user, void *in, size_t len);
#if 0
int
callback_demo(struct lws *wsi, enum lws_callback_reasons reason,
                  void *user, void *in, size_t len);
#endif

class Orcvlws {
    public:

    private:
    public:
        //============================
        // local websocket parameter
        //============================
        int    port;                                   // 通信端口
        struct lws_context *context;
        struct lws_context_creation_info context_info; // 用于填充创建vhost或者context时所需参数
        //===========================================================
        // if local websocket is client, need set service parameter
        //===========================================================
        const char* addr;                              // 客户端要连接的服务端地址
        struct lws *conn_wsi;                          // 客户端与服务端建立的连接
        struct lws_client_connect_info conn_info;      // 客户端要连接的服务端信息结构体


    public:
        Orcvlws() = delete;
        Orcvlws(uint32_t port, struct lws_protocols protocols[]);                        // as service
        Orcvlws(uint32_t port, const char *addr, struct lws_protocols protocols[]);      // as client
        ~Orcvlws();

        int  CreateLocalContext();
        int  ConnService();
        void WaitClient();
        void WaitService();
        void SetSignal();                              // 用于临时退出

    private:
};

#endif
