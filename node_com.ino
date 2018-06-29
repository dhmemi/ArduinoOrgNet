/*节点相关宏定义*/
#define NID  04
//节点编号，nodeID
#define NCT  0    //控制节点
#define NBC  255  //广播号
#define NSZ  5    //允许的最大节点数，nodeSize
#define FML  64   //协议栈最大帧长度

#define MXTMR 8  //链路状态计时器最大值
#define BUFSZ 128 //读数据缓冲区长度

enum CMD{
  Broadcast = 0xA5, //广播路由表作为心跳包
  SendData  = 0x01, //数据传输
  LinkCtrl  = 0x30, //路由表控制命令
  SendCtrl = 0x50,  //控制发送
};

struct RouteNode{
  bool ctrlAble;  //链路控制器
  byte timer;     //链路状态计数器
};

//路由树节点
struct SearchNode{
  byte nodeId;
  SearchNode *parent;
  SearchNode *bother;
  SearchNode *sons;
  SearchNode(byte id){
    nodeId = id;
    parent = NULL;
    bother = NULL;
    sons = NULL;
  }
};

RouteNode RouteTable[NSZ][NSZ];  //路由表记录

byte FrameNo = 0;  //frame No
byte RCtr = 0; //网络层控制字
byte MACCtr = 0; //MAC控制字

byte readBuff[BUFSZ];
byte buffLength;

void setup() {
  //初始化路由表和路由控制表
  for(int i = 0; i < NSZ; ++i){
    for(int j = 0; j < NSZ; ++j){
      RouteTable[i][j].ctrlAble = true;
      RouteTable[i][j].timer = 0;
    }
  }
  buffLength = 0;
  Serial.begin(9600);//设置波特率
}

void loop() {
  for(int i = 0; i < 4; i++){
  while(receive());
  delay(500);
  }
  
  routeCount();
  broadcast();
  delay(2000);
}

void broadcast()
{
  byte arr[NSZ*NSZ];
  for(int i = 0; i < NSZ; ++i){
    for(int j = 0; j < NSZ; ++j){
      if(RouteTable[i][j].ctrlAble){
        arr[i*NSZ+j] = RouteTable[i][j].timer;
      }else{
        arr[i*NSZ+j] = 0;
      }
    }
  }
  sendTo(arr,NSZ*NSZ, NBC, CMD::Broadcast);
}

bool sendTo(byte *data, byte Rlen, byte NDS, byte cmd)
{
  byte RFrame[56];
  RFrame[0] = NID;    //相当于ip协议中的源ip地址
  RFrame[1] = NDS;    //相当于ip协议中的目的ip地址
  RFrame[2] = RCtr;   //网络层控制字
  RFrame[3] = cmd;    //命令
  for(int i = 0; i < Rlen; ++i){
    RFrame[4+i] = data[i];
  }
  byte NodeDst = findWay(NDS);
  if(NodeDst == byte(0xFE)){
    return false;
  }
  structFrame(RFrame, Rlen+4, NodeDst);
  return true;
}

byte findWay(byte NDS)
{
  if(NDS == NBC || NDS == NCT){
    return NDS;
  }else{
    byte dnode = search(NDS);
    if(dnode == byte(0xFE)){
      byte info[] = "[Error]:0 connot find way to 0\n";
      info[8] = '0'+ NID;
      info[29] = '0'+NDS;
      sendTo(info, 31, 0, CMD::SendData);
    }
    return dnode;
  }  
  return byte(0xFE);
}

bool structFrame(byte *data, byte len, byte NodeDst)
{
  if(len > 56){
    return false;
  }
  byte frame[64];
  frame[0] = 0xFE;                //帧头
  frame[1] = NID;                 //相当于源MAC地址
  frame[2] = NodeDst;             //相当于目的MAC地址
  frame[3] = FrameNo;             //帧号高位
  frame[4] = MACCtr;              //控制字
  frame[5] = len;                 //帧长
  for(int i = 0; i < len; ++i){
    frame[6+i] = data[i];
  }
  frame[6+len] = getCheck(frame, len+6);

  while(receive());
  Serial.write(frame, len+7);
  
  ++ FrameNo;
  return true;
}

byte getCheck(byte *frame, byte len)
{
  byte checkdata = 0;
  for(int i = 0; i < len; ++i){
    checkdata += frame[i];
  }
  return (~checkdata);
}

bool receive()
{
  int len = Serial.available();
  if(len > 0){
    Serial.readBytes(&(readBuff[buffLength]), len);
    buffLength += len;
    parseBuf();
    return true;
  }
  return false;
}

//从缓冲区中寻找一个帧
void parseBuf()
{
  byte *frame;
  int frameBegin = 0, frameLength = 0;
  while(frameBegin + 5 < buffLength &&  readBuff[frameBegin] != 0xFE) { frameBegin++; }
  if(frameBegin + 5 < buffLength){
    frameLength = readBuff[frameBegin+5]+7;
    if(frameBegin+frameLength <= BUFSZ){
      frame = new byte[frameLength];
      memcpy(frame, readBuff, frameLength);
      frameBegin += frameLength;
    }
  }
  
  cleanBuff(frameBegin); 
  parseFrame(frame, frameLength); //帧内数据解析
  delete frame;
}

//清理读数据缓冲区，删除已被解析的数据
void cleanBuff(int frameBegin)  
{
  buffLength -= frameBegin;
  memcpy(readBuff, &(readBuff[frameBegin]), buffLength);
}

void parseFrame(byte *frame, int frameLength)
{
  if(frame[2] != NID && frame[2] != NBC){
    //MAC目的地址不是本节点目的地址或广播地址就忽略
    return;
  }else if(frame[frameLength-1] != getCheck(frame, frameLength-1)){
    //byte info[] = "[Error]:0 check error!";
    //info[8] = '0'+ NID;
    //sendTo(info, 22, 0, CMD::SendData);
    return;
  }
  
  switch(frame[9]){
  case CMD::Broadcast:
    rcvBC(frame, frameLength);
    break;
  case CMD::SendData:
    rcvSD(frame, frameLength);
    break;
  case CMD::LinkCtrl:
    rcvCN(frame, frameLength);
    break;
  case CMD::SendCtrl:
    rcvNT(frame, frameLength);
    break;
  }
}

void rcvBC(byte *frame, int frameLength)
{
  if(RouteTable[NID-1][frame[1]-1].ctrlAble){
    RouteTable[NID-1][frame[1]-1].timer = MXTMR;
    RouteTable[frame[1]-1][NID-1].timer = MXTMR;
  }
  for(int i = 0; i < NSZ; ++i){
    for(int j = 0; j < NSZ; ++j){
      if(frame[i*NSZ+j+10] > RouteTable[i][j].timer){
        RouteTable[i][j].timer = frame[i*NSZ+j+10];
      }
    }
  }
}

void rcvSD(byte *frame, int frameLength)
{
  if(frame[7] == NID){
    //如果本机就是目的节点，向控制主机报告收到数据
    byte *info = new byte[13+frame[5]];
    memcpy(info, "[info]:0 rcv:", 13);
    memcpy(&(info[13]), &(frame[9]), frame[5]);
    info[7] = '0'+NID;
    sendTo(info, 13+frame[5], 0, SendData);
    delete info;
  }else{
    byte NDS = findWay(frame[7]);
    if(NDS != -1){
      structFrame(&(frame[6]), frame[5], NDS);
    }
  }
}

void rcvCN(byte *frame, int frameLength)
{
  for(int i = 0; i < NSZ; ++i){
    for(int j = 0; j < NSZ; ++j){
      RouteTable[i][j].ctrlAble = bool(frame[i*NSZ+j+10]);
      if(!RouteTable[i][j].ctrlAble){
        RouteTable[i][j].timer = 0;
      }
    }
  }
}

void rcvNT(byte *frame, int frameLength)
{
  sendTo(&(frame[11]), frame[5]-5, frame[10], CMD::SendData);
}

byte search(byte target)
{
  SearchNode *root = new SearchNode(NID);
  byte result = doSearch(root, target);
  clearTree(root);
  return result;
}

byte doSearch(SearchNode *parent, byte target)
{
  byte result = 0xFE;
  SearchNode *frontBother = NULL;
  for(int i = 0; i < NSZ; ++i){
    
    //判断路由链路是否可达
    if(RouteTable[parent->nodeId-1][i].timer>0 && RouteTable[parent->nodeId-1][i].ctrlAble){

      //环路检测，防止出现环路
      bool flag = true;
      for(SearchNode *tmp = parent; tmp != NULL && flag; tmp = tmp->parent){
        if(tmp->nodeId == i+1){
          flag = false;
        }
      }

      //加入子/兄节点
      if(flag){
        SearchNode *tmp = new SearchNode(i+1);
        tmp->parent = parent;
        if(frontBother == NULL){
          parent->sons = tmp;
        }else{
          frontBother->bother = tmp;
        }
        frontBother = tmp;
      }
    }
  }

  //搜索子节点
  for(frontBother = parent->sons; frontBother != NULL; frontBother = frontBother->bother){
    if(frontBother->nodeId == target){
      result = getNextNode(frontBother);
    }else{
      result = doSearch(frontBother, target);
    }
    if(result != byte(0xFE)){
      break;
    }
  }
  return result;
}

//返回下一跳目标节点
byte getNextNode(SearchNode *leaf)
{
  SearchNode *tmp  = leaf;
  while(tmp->parent->parent != NULL){
    tmp = tmp->parent;
  }
  return tmp->nodeId;
}

void clearTree(SearchNode *parent)
{
  for(SearchNode *frontBother = parent->sons; frontBother != NULL; frontBother = frontBother->bother){
    clearTree(frontBother);
  }
  delete parent;
}

void routeCount()
{
  //若链路计时器到0，认为链路已断开
  for(int i = 0; i < NSZ; ++i){
    for(int j = 0; j < NSZ; ++j){
      if(RouteTable[i][j].timer > 0){
        RouteTable[i][j].timer -= 1;
      }
    }
  }
}

void serialEvent()
{
  receive();
}

