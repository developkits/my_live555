/**
������ͬʱ�ṩ�������鲥���ܡ�����testH264VideoStreamer�����޸ģ���ο�testOnDemandRTSPServer��
ע��
�������ؿ�VLC���ӣ������¶��ļ�����������
�鲥���ؿ�VLC���ӣ��������һ�ε�λ�����¶��ļ���ÿ������ʱ�����������ˣ�VLC���֣�
      main error: pictures leaked, trying to workaround

*/
#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>

UsageEnvironment* env;
char inputFileName[128] = {0};  // �������Ƶ�ļ�
H264VideoStreamFramer* videoSource;
RTPSink* videoSink;

Boolean reuseFirstSource = False;

void play(); // forward

void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
			   char const* streamName, char const* inputFileName);

int main(int argc, char** argv) {
  strcpy(inputFileName, "test.264"); // Ĭ��ֵ
  if (argc == 2) {
    strcpy(inputFileName, argv[1]);
  }
  LL_DEBUG(6, "Using file: %s\n", inputFileName);
  
  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  env = BasicUsageEnvironment::createNew(*scheduler);

  // ������Ϣ
  char const* descriptionString
    = "Session streamed by \"testH264VideoStreamer\"";

  // RTSP���������˿�Ϊ8554
  RTSPServer* rtspServer = RTSPServer::createNew(*env, 8554);
  if (rtspServer == NULL) {
    *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
    exit(1);
  }

	OutPacketBuffer::maxSize = 200000;
#if 0
  // �鲥
  // note ��mingw/linux�������鲥���ִ�����ϡ�{}������������ʱ����ԭ��δ֪
  // Create 'groupsocks' for RTP and RTCP:
  struct in_addr destinationAddress;
  destinationAddress.s_addr = chooseRandomIPv4SSMAddress(*env);

  const unsigned short rtpPortNum = 18888;
  const unsigned short rtcpPortNum = rtpPortNum+1;
  const unsigned char ttl = 255;

  const Port rtpPort(rtpPortNum);
  const Port rtcpPort(rtcpPortNum);

  Groupsock rtpGroupsock(*env, destinationAddress, rtpPort, ttl);
  rtpGroupsock.multicastSendOnly(); // we're a SSM source
  Groupsock rtcpGroupsock(*env, destinationAddress, rtcpPort, ttl);
  rtcpGroupsock.multicastSendOnly(); // we're a SSM source

  // Create a 'H264 Video RTP' sink from the RTP 'groupsock':
  
  videoSink = H264VideoRTPSink::createNew(*env, &rtpGroupsock, 96);

  // Create (and start) a 'RTCP instance' for this RTP sink:
  const unsigned estimatedSessionBandwidth = 500; // in kbps; for RTCP b/w share
  const unsigned maxCNAMElen = 100;
  unsigned char CNAME[maxCNAMElen+1];
  gethostname((char*)CNAME, maxCNAMElen);
  CNAME[maxCNAMElen] = '\0'; // just in case
  RTCPInstance* rtcp
  = RTCPInstance::createNew(*env, &rtcpGroupsock,
			    estimatedSessionBandwidth, CNAME,
			    videoSink, NULL /* we're a server */,
			    True /* we're a SSM source */);
  // Note: This starts RTCP running automatically

  char const* streamName = "h264ESVideoMulticast";
  ServerMediaSession* sms
    = ServerMediaSession::createNew(*env, streamName, inputFileName,
		   descriptionString, True /*SSM*/);
  sms->addSubsession(PassiveServerMediaSubsession::createNew(*videoSink, rtcp));
  rtspServer->addServerMediaSession(sms);

  announceStream(rtspServer, sms, streamName, inputFileName);

  // Start the streaming:
  //*env << "Beginning streaming...\n";
  rtspLog("Beginning streaming...\n");
  play(); // ����
#endif
  ////////////////////////////////////////////////////////////////////////

#if 01
  // ����
  {
    char const* streamName = "h264ESVideo";
    ServerMediaSession* sms
      = ServerMediaSession::createNew(*env, streamName, streamName,
				      descriptionString);
    sms->addSubsession(H264VideoFileServerMediaSubsession
		       ::createNew(*env, inputFileName, reuseFirstSource));
    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName, inputFileName);
  }
#endif
  printf("[%s %d] debug: before doEventLoop\n", __func__, __LINE__);
  env->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
}

// ������ȡ�ļ�
void afterPlaying(void* /*clientData*/) {
  *env << "...done reading from file\n";
  videoSink->stopPlaying();
  Medium::close(videoSource);
  // Note that this also closes the input file that this source read from.

  // Start playing once again:
  play();
}

void play() {
  // Open the input file as a 'byte-stream file source':
  ByteStreamFileSource* fileSource
    = ByteStreamFileSource::createNew(*env, inputFileName);
  if (fileSource == NULL) {
    *env << "Unable to open file \"" << inputFileName
         << "\" as a byte-stream file source\n";
    exit(1);
  }

  FramedSource* videoES = fileSource;

  // Create a framer for the Video Elementary Stream:
  videoSource = H264VideoStreamFramer::createNew(*env, videoES);

  // Finally, start playing:
  *env << "Beginning to read from file...\n";
  videoSink->startPlaying(*videoSource, afterPlaying, videoSink);
}

void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
			   char const* streamName, char const* inputFileName) {
  char* url = rtspServer->rtspURL(sms);
  UsageEnvironment& env = rtspServer->envir();
  env << "\n\"" << streamName << "\" stream, from the file \""
      << inputFileName << "\"\n";
  env << "Play this stream using the URL \"" << url << "\"\n";
  delete[] url;
}