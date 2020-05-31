// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace CrystalAEPUI
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MainUI());
        }
    }
}
