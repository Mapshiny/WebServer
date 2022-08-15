#include "burger/net/Scheduler.h"
#include "burger/net/CoTcpServer.h"
#include "burger/base/Log.h"
#include <stdio.h>
#include <unistd.h>

using namespace burger;
using namespace burger::net;

const int kBufSize = 64*1024;  // 64K
const char* g_file = NULL;

void connHandler(const CoTcpConnectionPtr& conn) {
    INFO("FileServer - Sending file {} to {}", g_file, conn->getPeerAddr().getIpPortStr());
    FILE* fp = ::fopen(g_file, "rb");
    if (fp) {
        char buf[kBufSize];
        size_t nread = 0;
        do {
            nread = ::fread(buf, 1, sizeof buf, fp);
            conn->send(buf, nread);
        } while(nread > 0);
        ::fclose(fp);
        fp = nullptr;
        conn->shutdown();
        INFO("FileServer - done");
    } else {
        conn->shutdown();
        INFO("FileServer - no such file");
    }
}

int main(int argc, char* argv[]) {

    if (argc > 1) {
        g_file = argv[1];

        Scheduler sched;
        InetAddress listenAddr(8888);
        CoTcpServer server(&sched, listenAddr, "FileServer");
        server.setConnectionHandler(connHandler);
        server.start();
        sched.wait();
    }
    else {
        fprintf(stderr, "Usage: %s file_for_downloading\n", argv[0]);
    }
}