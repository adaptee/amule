using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace amule.net
{
    public partial class ConnectDlg : Form
    {
        public ConnectDlg()
        {
            InitializeComponent();
        }

        private void ConnectDlg_Load(object sender, EventArgs e)
        {

        }
        public string Host()
        {
            return amuleHost.Text;
        }
        public string Pass()
        {
            return amulePwd.Text;
        }
        public string Port()
        {
            return amulePort.Text;
        }
    }
}