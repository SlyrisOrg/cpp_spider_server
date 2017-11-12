//
// Created by doom on 10/11/17.
//

#include <gtest/gtest.h>
#include <Protocol/Messages.hpp>
#include <Protocol/CommandHandler.hpp>

using namespace spi;

static CommandHandler _handler;

TEST(Serialization, ReplyCode)
{
    proto::ReplyCode rep;

    rep.code = proto::ReplyType::OK;

    Buffer buf;
    rep.serialize(buf);
    ASSERT_EQ(buf.size(), Serializable::HeaderSize + proto::ReplyCode::SerializedSize);
    buf.erase(buf.begin(), buf.begin() + Serializable::MetaDataSize);

    ASSERT_EQ(_handler.identifyMessage(buf), proto::MessageType::ReplyCode);

    proto::ReplyCode result;
    result << buf;
    ASSERT_EQ(result.code, proto::ReplyType::OK);
}

TEST(Serialization, Bye)
{
    proto::Bye bye;

    Buffer buf;
    bye.serialize(buf);
    ASSERT_EQ(buf.size(), Serializable::HeaderSize + proto::Bye::SerializedSize);
    buf.erase(buf.begin(), buf.begin() + Serializable::MetaDataSize);

    ASSERT_EQ(_handler.identifyMessage(buf), proto::MessageType::Bye);
//    proto::Bye result;
}

TEST(Serialization, RawData)
{
    std::string data = "this is a test";

    proto::RawData raw;

    raw.bytes.insert(raw.bytes.begin(), data.begin(), data.end());

    Buffer buf;
    raw.serialize(buf);
    ASSERT_EQ(buf.size(), Serializable::HeaderSize + proto::RawData::SerializedSize + data.size());
    buf.erase(buf.begin(), buf.begin() + Serializable::MetaDataSize);

    ASSERT_EQ(_handler.identifyMessage(buf), proto::MessageType::RawData);

    proto::RawData result;
    result << buf;
    ASSERT_EQ(result.bytes.size(), raw.bytes.size());
    std::string resultStr(result.bytes.begin(), result.bytes.end());
    ASSERT_EQ(data, resultStr);
}

TEST(Serialization, Hello)
{
    proto::Hello h;

    h.macAddress.get();
    h.md5 = utils::MD5("this is a test");
    h.port = 12345;
    h.version = 1;

    Buffer buf;
    h.serialize(buf);
    ASSERT_EQ(buf.size(), Serializable::HeaderSize + proto::Hello::SerializedSize);
    buf.erase(buf.begin(), buf.begin() + Serializable::MetaDataSize);

    ASSERT_EQ(_handler.identifyMessage(buf), proto::MessageType::Hello);

    proto::Hello result;
    result << buf;
    ASSERT_EQ(h.macAddress.raw(), result.macAddress.raw());
    ASSERT_EQ(h.md5.raw(), result.md5.raw());
    ASSERT_EQ(h.port, result.port);
    ASSERT_EQ(h.version, result.version);
}

TEST(Serialization, KeyEvent)
{
    proto::KeyEvent ke;

    ke.code = proto::KeyCode::M;
    ke.state = proto::KeyState::Down;
    ke.timestamp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());

    Buffer buf;
    ke.serialize(buf);
    ASSERT_EQ(buf.size(), Serializable::HeaderSize + proto::KeyEvent::SerializedSize);
    buf.erase(buf.begin(), buf.begin() + Serializable::MetaDataSize);

    ASSERT_EQ(_handler.identifyMessage(buf), proto::MessageType::KeyEvent);

    proto::KeyEvent result;
    result << buf;
    ASSERT_EQ(ke.code, result.code);
    ASSERT_EQ(ke.state, result.state);
    ASSERT_EQ(ke.timestamp.time_since_epoch().count(), result.timestamp.time_since_epoch().count());
}

TEST(Serialization, MouseClick)
{
    proto::MouseClick mc;

    mc.button = proto::MouseButton::Left;
    mc.state = proto::KeyState::Down;
    mc.timestamp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    mc.x = 23;
    mc.y = 32;

    Buffer buf;
    mc.serialize(buf);
    ASSERT_EQ(buf.size(), Serializable::HeaderSize + proto::MouseClick::SerializedSize);
    buf.erase(buf.begin(), buf.begin() + Serializable::MetaDataSize);

    ASSERT_EQ(_handler.identifyMessage(buf), proto::MessageType::MouseClick);

    proto::MouseClick result;
    result << buf;
    ASSERT_EQ(mc.button, result.button);
    ASSERT_EQ(mc.state, result.state);
    ASSERT_EQ(mc.timestamp.time_since_epoch().count(), result.timestamp.time_since_epoch().count());
    ASSERT_EQ(mc.x, result.x);
    ASSERT_EQ(mc.y, result.y);
}

TEST(Serialization, MouseMove)
{
    proto::MouseMove mc;

    mc.timestamp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    mc.x = 23;
    mc.y = 32;

    Buffer buf;
    mc.serialize(buf);
    ASSERT_EQ(buf.size(), Serializable::HeaderSize + proto::MouseMove::SerializedSize);
    buf.erase(buf.begin(), buf.begin() + Serializable::MetaDataSize);

    ASSERT_EQ(_handler.identifyMessage(buf), proto::MessageType::MouseMove);

    proto::MouseMove result;
    result << buf;
    ASSERT_EQ(mc.timestamp.time_since_epoch().count(), result.timestamp.time_since_epoch().count());
    ASSERT_EQ(mc.x, result.x);
    ASSERT_EQ(mc.y, result.y);
}


TEST(Serialization, ImageData)
{
    std::string data = "this is a test";

    proto::ImageData raw;

    raw.bytes.insert(raw.bytes.begin(), data.begin(), data.end());

    Buffer buf;
    raw.serialize(buf);
    ASSERT_EQ(buf.size(), Serializable::HeaderSize + proto::ImageData::SerializedSize + data.size());
    buf.erase(buf.begin(), buf.begin() + Serializable::MetaDataSize);

    ASSERT_EQ(_handler.identifyMessage(buf), proto::MessageType::ImageData);

    proto::ImageData result;
    result << buf;
    ASSERT_EQ(result.bytes.size(), raw.bytes.size());
    std::string resultStr(result.bytes.begin(), result.bytes.end());
    ASSERT_EQ(data, resultStr);
}

TEST(Serialization, StealthMode)
{
    proto::StealthMode st;

    Buffer buf;
    st.serialize(buf);
    ASSERT_EQ(buf.size(), Serializable::HeaderSize + proto::StealthMode::SerializedSize);
    buf.erase(buf.begin(), buf.begin() + Serializable::MetaDataSize);

    ASSERT_EQ(_handler.identifyMessage(buf), proto::MessageType::StealthMode);
}

TEST(Serialization, ActiveMode)
{
    proto::ActiveMode ac;

    Buffer buf;
    ac.serialize(buf);
    ASSERT_EQ(buf.size(), Serializable::HeaderSize + proto::ActiveMode::SerializedSize);
    buf.erase(buf.begin(), buf.begin() + Serializable::MetaDataSize);

    ASSERT_EQ(_handler.identifyMessage(buf), proto::MessageType::ActiveMode);
}

TEST(Serialization, RList)
{
    proto::RList rl;

    Buffer buf;
    rl.serialize(buf);
    ASSERT_EQ(buf.size(), Serializable::HeaderSize + proto::RList::SerializedSize);
    buf.erase(buf.begin(), buf.begin() + Serializable::MetaDataSize);

    ASSERT_EQ(_handler.identifyMessage(buf), proto::MessageType::RList);
}

TEST(Serialization, WindowChanged)
{
    proto::WindowChanged raw;
    raw.windowName = "this is a test";

    Buffer buf;
    raw.serialize(buf);
    ASSERT_EQ(buf.size(), Serializable::HeaderSize + proto::WindowChanged::SerializedSize + raw.windowName.size());
    buf.erase(buf.begin(), buf.begin() + Serializable::MetaDataSize);

    ASSERT_EQ(_handler.identifyMessage(buf), proto::MessageType::WindowChange);

    proto::WindowChanged result;
    result << buf;
    ASSERT_EQ(result.windowName, raw.windowName);
}

TEST(Serialization, RunShell)
{
    proto::RunShell rsh;
    rsh.cmd = "ls -la";

    Buffer buf;
    rsh.serialize(buf);
    ASSERT_EQ(buf.size(), Serializable::HeaderSize + proto::RunShell::SerializedSize + rsh.cmd.size());
    buf.erase(buf.begin(), buf.begin() + Serializable::MetaDataSize);

    ASSERT_EQ(_handler.identifyMessage(buf), proto::MessageType::RunShell);

    proto::RunShell result;
    result << buf;
    ASSERT_EQ(result.cmd, rsh.cmd);
}

TEST(Serialization, RRunShell)
{
    proto::RRunShell rrsh;
    rrsh.target.get();
    rrsh.cmd = "ls -la";

    Buffer buf;
    rrsh.serialize(buf);
    ASSERT_EQ(buf.size(), Serializable::HeaderSize + proto::RRunShell::SerializedSize + rrsh.cmd.size());
    buf.erase(buf.begin(), buf.begin() + Serializable::MetaDataSize);

    ASSERT_EQ(_handler.identifyMessage(buf), proto::MessageType::RRunShell);

    proto::RRunShell result;
    result << buf;
    ASSERT_EQ(result.target, rrsh.target);
    ASSERT_EQ(result.cmd, rrsh.cmd);
}

TEST(Serialization, Empty)
{
    Buffer empty;

    proto::ReplyCode code;
    ASSERT_THROW(code.unserialize(empty), UnserializationError);

    proto::RawData rd;
    ASSERT_THROW(rd.unserialize(empty), UnserializationError);

    proto::Hello hello;
    ASSERT_THROW(hello.unserialize(empty), UnserializationError);

    proto::KeyEvent ke;
    ASSERT_THROW(hello.unserialize(empty), UnserializationError);

    proto::MouseClick mc;
    ASSERT_THROW(mc.unserialize(empty), UnserializationError);

    proto::MouseMove mm;
    ASSERT_THROW(mm.unserialize(empty), UnserializationError);

    proto::ImageData id;
    ASSERT_THROW(id.unserialize(empty), UnserializationError);
}

TEST(ProtocolMessage, MessageIdentification)
{
    CommandHandler cmdHandler;
    Buffer buf{0x01, 0x01, 0x01, 0x01};

    ASSERT_EQ(cmdHandler.identifyMessage(buf), proto::MessageType::Unknown);

    buf = cmdHandler.makeHeader(proto::MessageType::Hello);
    ASSERT_EQ(cmdHandler.identifyMessage(buf), proto::MessageType::Hello);
}
