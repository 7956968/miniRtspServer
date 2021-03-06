/*************************************************************************
Copyright (c) 2020 Taoist Luo

Create by: Taoist Luo
CSDN：https://blog.csdn.net/daichanzhang9734/article/details/107549026

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*****************************************************************************/
#include <arpa/inet.h>
#include "RtpSink.h"
#include "Mylog.h"
RtpSink::RtpSink(Env* env,MediaSource* mediaSource, int payloadType) :
    mMediaSource(mediaSource),
    mSendPacketCallback(NULL),
    mCsrcLen(0),
    mExtension(0),
    mPadding(0),
    mVersion(RTP_VESION),
    mPayloadType(payloadType),
    mMarker(0),
    mSeq(0),
    mTimestamp(0),
    mEnv(env)
    
{
    uint32_t frameTime = 1000/(mMediaSource->getFps());
    mEnv->addTimer(timeoutCallback,frameTime,true,this);
    mSSRC = rand();
}

RtpSink::~RtpSink()
{
    delete mTimerEvent;
}

void RtpSink::setSendFrameCallback(SendPacketCallback cb, void* arg1, void* arg2)
{
    mSendPacketCallback = cb;
    mArg1 = arg1;
    mArg2 = arg2;
}

void RtpSink::sendRtpPacket(RtpPacket* packet)
{
    RtpHeader* rtpHead = packet->mRtpHeadr;
    rtpHead->csrcLen = mCsrcLen;
    rtpHead->extension = mExtension;
    rtpHead->padding = mPadding;
    rtpHead->version = mVersion;
    rtpHead->payloadType = mPayloadType;
    rtpHead->marker = mMarker;
    rtpHead->seq = htons(mSeq);
    rtpHead->timestamp = htonl(mTimestamp);
    rtpHead->ssrc = htonl(mSSRC);
    packet->mSize += RTP_HEADER_SIZE;
    
    if(mSendPacketCallback)
        mSendPacketCallback(mArg1, mArg2, packet);
}

void RtpSink::timeoutCallback(void* arg)
{
    RtpSink* rtpSink = (RtpSink*)arg;
    AVFrame* frame = rtpSink->mMediaSource->getFrame();
    if(!frame)
    {
        LOGI("get frame was NULL\n");
        return;
    }

    rtpSink->handleFrame(frame);

    rtpSink->mMediaSource->putFrame(frame);
    
}

void RtpSink::start(int ms)
{
    //mTimerEvent->setTimeout((uint32_t)ms);
    mTimerEvent->start(ms*1000,true);
}

void RtpSink::stop()
{
    //mEnv->scheduler()->removeTimedEvent(mTimerId);
}