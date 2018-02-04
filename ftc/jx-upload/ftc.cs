using System;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace jx_upload
{
    class ftc
    {
        protected Socket client;
        public ftc(string sIp, CallBackFun fun) : this(sIp, 5209, fun)
        {

        }
        public ftc(string sIp, int port = 5209, CallBackFun fun = null)
        {
            if (fun != null)
            {
                SetCallBack(fun);
            }
            try
            {
                IPAddress ip = IPAddress.Parse(sIp);
                IPEndPoint ipe = new IPEndPoint(ip, port);
                client = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                client.Connect(ipe);
            }
            catch (Exception)
            {
                backFun(-1, "连接失败");
                return;
            }
            Thread thread = new Thread(new ThreadStart(RecvThread));
            thread.Start();
        }
        ~ftc()
        {
            client.Close();
        }

        public delegate void CallBackFun(int code, string msg);
        CallBackFun backFun = null;
        public void SetCallBack(CallBackFun fun)
        {
            this.backFun = fun;
        }

        public void RecvThread()
        {
            try
            {
                while (client.Connected)
                {
                    byte[] buff = new byte[1024];
                    int length=0;
                    try
                    {
                         length = client.Receive(buff, buff.Length, 0);
                    }
                    catch (Exception)
                    {
                        break;
                    }
                    if (length > 0)
                    {
                        int code = buff[0];
                        buff = buff.Skip(1).ToArray();
                        string data = Encoding.UTF8.GetString(buff).TrimEnd('\0');
                        backFun(code, data);
                        if (code != 10 && code != 0)
                        {
                            client.Close();
                        }
                        else if (code == 10)
                        {
                            //开始发送文件数据
                            long sendTotalLen = 0;
                            string tmpStrSendLen = Functions.ReadIni("sid_" + id.ToString(), "send");
                            if (tmpStrSendLen != "")
                            {
                                sendTotalLen = long.Parse(tmpStrSendLen)-8192;
                                if (sendTotalLen<0)
                                {
                                    sendTotalLen = 0;
                                }
                            }
                            FileStream file = File.Open(filePath, FileMode.Open, FileAccess.Read);
                            file.Seek(sendTotalLen,SeekOrigin.Begin);
                            Functions.WriteIni("sid_" + id.ToString(), "tmp", data);
                            while (sendTotalLen < fileSize)
                            {
                                int sendLen = 0;
                                if (sendTotalLen + 524288 > fileSize)
                                {
                                    sendLen = (int)(fileSize - sendTotalLen);
                                }
                                else
                                {
                                    sendLen = 524288;
                                }
                                byte[] buffer = new byte[sendLen];
                                sendLen = file.Read(buffer, 0, sendLen);
                                sendLen = Send(buffer, sendLen);
                                sendTotalLen += sendLen;
                                backFun(1000, ((int)(((double)sendTotalLen / (double)fileSize) * 100)).ToString());
                                Functions.WriteIni("sid_"+id.ToString(),"send", sendTotalLen.ToString());
                            }
                            file.Close();
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
            catch (IOException ex)
            {
                var tmp = ex.Message;
            }


        }

        internal void SendToSid()
        {
            this.filePath = Functions.ReadIni("sid_"+ id.ToString(),"filename" );
            FileInfo fileInfo = new FileInfo(filePath);
            fileSize = (long)fileInfo.Length;
            var sizeByte = BitConverter.GetBytes(fileSize);
            sizeByte = BitConverter.GetBytes(fileSize);
            sizeByte = sizeByte.Concat(BitConverter.GetBytes(id)).ToArray();
            string sLen = Functions.ReadIni("sid_" + id.ToString(), "send");
            long tmpSend=0;
            if (sLen != "")
            {
                tmpSend = long.Parse(sLen) - 8192;
                if (tmpSend < 0)
                {
                    tmpSend = 0;
                }
            }
            Byte[] tmpBuff = BitConverter.GetBytes(tmpSend);
            tmpBuff = sizeByte.Concat(tmpBuff).ToArray();
            sizeByte=Encoding.UTF8.GetBytes(Functions.ReadIni("sid_" + id.ToString(), "tmp"));
            tmpBuff = tmpBuff.Concat(sizeByte).ToArray();
            Byte[] code = { 2 };
            tmpBuff = code.Concat(tmpBuff).ToArray();
            if (Send(tmpBuff, tmpBuff.Length) <= 0)
            {
                backFun(-1, "发送失败");
            }
        }

        private long id;
        public void SetFileId(long id)
        {
            this.id = id;
        }
        private string filePath;
        private long fileSize;
        public void SendFileMsg(string filePath)
        {
            this.filePath = filePath;
            string fileName = Path2name(filePath);
            FileInfo fileInfo = new FileInfo(filePath);
            fileSize = (long)fileInfo.Length;
            var sizeByte = BitConverter.GetBytes(fileSize);
            sizeByte = BitConverter.GetBytes(fileSize);
            sizeByte = sizeByte.Concat(BitConverter.GetBytes(id)).ToArray();
            Byte[] tmpBuff = Encoding.UTF8.GetBytes(fileName);
            tmpBuff = sizeByte.Concat(tmpBuff).ToArray();
            Byte[] code = { 1 };
            tmpBuff = code.Concat(tmpBuff).ToArray();
            if (Send(tmpBuff, tmpBuff.Length) <= 0)
            {
                backFun(-1, "发送失败");
            }
            else
            {
                Functions.WriteIni("sid_"+id.ToString(),"filename",filePath);
            }
        }


        private string Path2name(string filePath)
        {
            string name = "";
            int pos = filePath.LastIndexOf("\\");
            if (pos < 0)
            {
                return "";
            }
            name = filePath.Substring(pos + 1);
            return name;
        }

        private int Send(byte[] buff, int sendLen)
        {
            int len;
            try
            {
                len = client.Send(buff, sendLen, 0);
            }
            catch (Exception)
            {
                return 0;
            }
            return len;
        }

    }
}
