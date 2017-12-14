using System;
using System.Windows.Forms;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System.IO;
using System.Reflection;

namespace jx_upload
{
    public partial class LoginForm : Form
    {
        public LoginForm()
        {
            InitializeComponent();
        }
        private Form1 mainForm;
        private void button1_Click(object sender, EventArgs e)
        {
            string retData = Functions.HttpGet(Functions.URL + "/index/index/login", "user=" + textBox1.Text + "&pwd=" + textBox2.Text);
            if (retData == "")
            {
                MessageBox.Show("登录超时,需要在校园网内使用");
                return;
            }
            JObject jo = (JObject)JsonConvert.DeserializeObject(retData);
            string zone = jo["code"].ToString();
            string zone_en = jo["msg"].ToString();
            MessageBox.Show(zone_en, "提示");
            if (zone == "0")
            {
                Functions.WriteIni("data", "user", textBox1.Text);
                Functions.WriteIni("check", "savep", "true");
                Functions.WriteIni("data", "pwd", textBox2.Text);
                Functions.User = textBox1.Text;
                Functions.Pwd = textBox2.Text;
                mainForm = new Form1();
                mainForm.Show();
                //Application.Current.MainWindow = mwin;
                //mwin.Show();
                //this.Close();
                Hide();
            }
        }

        private void LoginForm_Load(object sender, EventArgs e)
        {
            textBox1.Text = Functions.ReadIni("data", "user");
            textBox2.Text = Functions.ReadIni("data", "pwd");
            string filePath = Application.StartupPath + @"\Newtonsoft.Json.dll";
            if (!File.Exists(filePath))
            {
                Type type = MethodBase.GetCurrentMethod().DeclaringType;

                string _namespace = type.Namespace;
                Assembly _assembly = Assembly.GetExecutingAssembly();
                string resourceName = _namespace + ".Resources.Newtonsoft.Json.dll";
                Stream stream = _assembly.GetManifestResourceStream(resourceName);

                using (FileStream fs = new FileStream(filePath, FileMode.Create))
                {
                    int fsLen = (int)stream.Length;
                    byte[] heByte = new byte[fsLen];
                    int r = stream.Read(heByte, 0, heByte.Length);
                    string myStr = System.Text.Encoding.UTF8.GetString(heByte);
                    fs.Write(heByte, 0, fsLen);
                }
            }
        }

        private void linkLabel1_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            try
            {
                System.Diagnostics.Process.Start("explorer.exe", Functions.URL);
            }
            catch (Exception)
            {
                System.Diagnostics.Process.Start("iexplore.exe", Functions.URL);
            }

        }
    }
}
