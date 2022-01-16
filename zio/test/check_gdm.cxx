

#include "WireCellUtil/String.h"
#include "WireCellUtil/Exceptions.h"
#include "WireCellUtil/Configuration.h"

#include "zio/domo/client.hpp"
#include "zio/tens.hpp"
#include "zio/logging.hpp"
#include "zio/actor.hpp"

#include <iostream>
#include <memory>

using namespace WireCell;

namespace {
    std::string dump(const zio::Message &msg)
    {
        std::stringstream ss;
        ss << "zio.Message: ";
        ss << "ZIO" << msg.level() << msg.form() << msg.label();
        ss << " + [0x" << msg.origin() << "," << msg.granule() << "," << msg.seqno() << "]";
        ss << " + [" << msg.payload().str() << "]";
        return ss.str();
    }
    zio::Message zio_tens_msg()
    {
#define N2 20
        std::vector<size_t> shape = {2, 2, N2};
        float tensor[2][2][N2] = {0};
        const float *tensor1 = (float *) tensor;
        zio::Message msg(zio::tens::form);
        // Add an initial, unrelated message part just to make sure tens
        // respects it.
        msg.add(zio::message_t((char *) nullptr, 0));

        Configuration label;
        label[zio::tens::form]["metadata"] = {};
        auto &meta = label[zio::tens::form]["metadata"];
        meta["tick"] = 500;
        Json::FastWriter jwriter;
        msg.set_label(jwriter.write(label));

        zio::tens::append(msg, tensor1, shape);

        return msg;
    }
}  // namespace

int main()
{
    // client
    std::shared_ptr<zio::domo::Client> m_client;
    zmq::context_t ctx;
    zmq::socket_t sock(ctx, ZMQ_CLIENT);
    m_client = std::make_shared<zio::domo::Client>(sock, "tcp://localhost:5555");

    // simple msg
    auto msg = zio_tens_msg();
    std::cout << dump(msg) << "\n";

    zmq::multipart_t mmsg(msg.toparts());
    std::cout << "\nzmq::multipart_t: " << mmsg.size();
    std::cout << mmsg.str() << "\n";
    m_client->send("echo", mmsg);
    std::cout << "m_client->send ... OK\n";

    mmsg.clear();
    m_client->recv(mmsg);
    std::cout << "m_client->recv ... OK\n";

    msg.fromparts(mmsg);
    std::cout << "msg.fromparts ... OK\n";
    std::cout << dump(msg) << "\n";

    return 0;
}