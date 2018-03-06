using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.IO;
using System.Net;
using System.Threading;
using System.Windows.Forms;

namespace jx_upload
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }
        private ftc m_ftc;
        private string m_image;
        private bool isupload = false;
        private void button1_Click(object sender, EventArgs e)
        {
            if (isupload)
            {
                MessageBox.Show("正在上传,请等待");
                return;
            }
            OpenFileDialog openFileDialog = new OpenFileDialog();
            openFileDialog.Filter = "所有文件|*.*";
            openFileDialog.RestoreDirectory = true;
            openFileDialog.FilterIndex = 1;
            if (openFileDialog.ShowDialog() == DialogResult.OK)
            {
                string fName = openFileDialog.FileName;
                //先获取id
                if (listBox1.SelectedIndex < 0)
                {
                    MessageBox.Show("未选择上传分区");
                    return;
                }
                sortItem tmpItem = (sortItem)listBox1.Items[listBox1.SelectedIndex];
                string retData = Functions.HttpPost(Functions.URL + "/index/api/isUpload", "name=" + Functions.URLEncode(textBox1.Text) + "&filename=" + Functions.URLEncode(Path.GetFileName(fName))
                    + "&exp=" + Functions.URLEncode(textBox2.Text) + "&sort_id=" + tmpItem.ID + "&price=" + textBox3.Text + "&logo=" + Functions.URLEncode(m_image));
                JObject jo = (JObject)JsonConvert.DeserializeObject(retData);
                string msg = jo["msg"].ToString();
                string code = jo["code"].ToString();
                if (code == "0")
                {
                    retData = Functions.HttpPost(Functions.URL + "/index/api/applyUpload", "name=" + Functions.URLEncode(textBox1.Text) + "&filename=" + Functions.URLEncode(Path.GetFileName(fName))
                        + "&exp=" + Functions.URLEncode(textBox2.Text) + "&sort_id=" + tmpItem.ID + "&price=" + textBox3.Text + "&logo=" + Functions.URLEncode(m_image));
                    jo = (JObject)JsonConvert.DeserializeObject(retData);
                    uploadFile(fName, jo["id"].ToString());
                }
                else if (code == "-2")
                {
                    DialogResult dr = MessageBox.Show("你有一个未完成的任务,是否先将未完成的任务上传?\n点击是将先上传之前的文件,否将上一个任务删除,直接进行新任务的上传", "提示", MessageBoxButtons.YesNo);
                    if (dr == DialogResult.Yes)
                    {
                        string filePath = Functions.ReadIni("sid_" + jo["sid"].ToString(), "filename");
                        if (!File.Exists(filePath))
                        {
                            if (MessageBox.Show("在这台电脑上没有找到相应的上传记录,是否删除记录?", "提示", MessageBoxButtons.YesNo) == DialogResult.Yes)
                            {
                                Functions.HttpPost(Functions.URL + "/index/user/remove_upload?sid=" + jo["sid"].ToString());
                            }
                            return;
                        }
                        isupload = true;
                        label4.Text = "准备上传...";
                        IPHostEntry host = Dns.GetHostByName("d.icodef.com");
                        IPAddress ip = host.AddressList[0];
                        m_ftc = new ftc(ip.ToString(), FtcCallBack);//10.127.134.51
                        m_ftc.SetFileId(long.Parse(jo["sid"].ToString()));
                        m_ftc.SendToSid();
                    }
                    else
                    {
                        Functions.HttpPost(Functions.URL + "/index/user/remove_upload?sid=" + jo["sid"].ToString());
                        retData = Functions.HttpPost(Functions.URL + "/index/api/applyUpload", "name=" + Functions.URLEncode(textBox1.Text) + "&filename=" + Functions.URLEncode(Path.GetFileName(fName))
                            + "&exp=" + Functions.URLEncode(textBox2.Text) + "&sort_id=" + tmpItem.ID + "&price=" + textBox3.Text + "&logo=" + Functions.URLEncode(m_image));
                        jo = (JObject)JsonConvert.DeserializeObject(retData);
                        uploadFile(fName, jo["id"].ToString());
                    }
                }
                else
                {
                    MessageBox.Show(msg);
                }
            }
        }

        private void uploadFile(string filename, string sid)
        {
            isupload = true;
            label4.Text = "开始上传...";
            IPHostEntry host = Dns.GetHostByName("d.icodef.com");
            IPAddress ip = host.AddressList[0];
            m_ftc = new ftc(ip.ToString(), FtcCallBack);//10.127.134.51
            m_ftc.SetFileId(long.Parse(sid));
            m_ftc.SendFileMsg(filename);
        }

        private void FtcCallBack(int code, string msg)
        {
            Action act = () =>
            {

                if (code == 0)
                {
                    MessageBox.Show("上传成功!");
                    isupload = false;
                }
                else if (code == 1000)
                {
                    progressBar1.Value = int.Parse(msg);
                    label4.Text = "上传进度:" + msg + "%";
                }
                else
                {
                    label4.Text = msg;
                    isupload = false;
                }
            };
            label4.Invoke(act);
        }
        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            System.Environment.Exit(0);
        }

        private void pictureBox1_Click(object sender, EventArgs e)
        {
            OpenFileDialog openFileDialog = new OpenFileDialog();
            openFileDialog.Filter = "图片|*.jpg;*.jpge;*.bmp;*.png";
            openFileDialog.RestoreDirectory = true;
            openFileDialog.FilterIndex = 1;
            if (openFileDialog.ShowDialog() == DialogResult.OK)
            {
                try
                {
                    string retData = Functions.HttpPostFile(Functions.URL + "/index/api/upload_image", openFileDialog.FileName);
                    JObject jo = (JObject)JsonConvert.DeserializeObject(retData);
                    string msg = jo["msg"].ToString();
                    string code = jo["code"].ToString();
                    MessageBox.Show(this, msg, "提示");
                    if (code == "0")
                    {
                        m_image = jo["url"].ToString();
                        pictureBox1.Image = System.Drawing.Image.FromStream(Functions.HttpFile(Functions.URL + "/static/res/images/" + jo["url"].ToString()).BaseStream);
                    }
                }
                catch (Exception)
                {
                    MessageBox.Show("网络错误");
                }

            }
        }
        class sortItem
        {
            private string name;
            private string sid;
            public sortItem(string sid, string name)
            {
                this.name = name;
                this.sid = sid;
            }
            public string Name
            {
                get
                {
                    return this.name;
                }
                set
                {
                    this.name = value;
                }
            }
            public override string ToString()
            {
                return this.name;
            }
            public string ID
            {
                get
                {
                    return this.sid;
                }
                set
                {
                    this.sid = value;
                }
            }
        }
        private void Form1_Load(object sender, EventArgs e)
        {
            Thread thread = new Thread(init);
            thread.Start();
        }
        private void init()
        {
            try
            {
                string retData = Functions.HttpGet(Functions.URL + "/api/getSortList");
                JObject jo = (JObject)JsonConvert.DeserializeObject(retData);
                Action act = () =>
                {
                    label4.Text = "获取分区列表....";
                    listBox1.Items.Clear();
                    string msg = jo["msg"].ToString();
                    string code = jo["code"].ToString();
                    foreach (var item in jo["rows"])
                    {
                        sortItem sort = new sortItem(item["sid"].ToString(), item["name"].ToString());
                        listBox1.Items.Add(sort);
                    }
                    label4.Text = "分区列表获取完成";
                };
                listBox1.Invoke(act);

            }
            catch (Exception e)
            {
                MessageBox.Show(e.Message);
            }
        }
    }
}
