// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Windows.Forms;
using System.IO;

namespace CrystalAEPUI
{
    public static class XmlConfig
    {
        public static bool OpenConfig(string configFile)
        {
            XmlReaderSettings xmlSettings = new XmlReaderSettings();

            xmlSettings.IgnoreComments = true;
            xmlSettings.IgnoreProcessingInstructions = true;
            xmlSettings.IgnoreWhitespace = true;
            xmlSettings.ProhibitDtd = true;

            try
            {
                string xmlConfigData = File.ReadAllText(configFile);
                ms = new MemoryStream(Encoding.ASCII.GetBytes(xmlConfigData));
                xmlReader = new XmlTextReader(ms);
            }
            catch
            {
                return false;
            }

            configFilePath = configFile;

            return true;
        }

        public static string Path
        {
            get
            {
                return configFilePath;
            }
        }

        public static bool RefreshConfig()
        {
            if (configFilePath == null)
            {
                Close();
                return false;
            }

            Close();
            ClearConfig();

            if (!OpenConfig(configFilePath))
                return false;

            if (!ReadConfig())
            {
                Close();
                return false;
            }

            return true;
        }

        public static bool ReadConfig()
        {
            Stack<string> elements = new Stack<string>();

            try
            {
                ClearConfig();

                while (xmlReader.Read())
                {
                    switch (xmlReader.NodeType)
                    {
                        case XmlNodeType.Element:

                            bool isEmptyElement = false;

                            if (!String.IsNullOrEmpty(xmlReader.Name))
                            {
                                elements.Push(xmlReader.Name);
                                isEmptyElement = xmlReader.IsEmptyElement;
                            }

                            if (xmlReader.HasAttributes)
                            {
                                if (xmlReader.MoveToFirstAttribute())
                                {
                                    NameValuePair nvpair = new NameValuePair();
                                    
                                    do
                                    {
                                        if (xmlReader.Name == "name")
                                            nvpair.Name = xmlReader.Value;

                                        if (xmlReader.Name == "value")
                                            nvpair.Value = xmlReader.Value;
                                    }
                                    while (xmlReader.MoveToNextAttribute());

                                    if (nvpair.IsValid())
                                    {
                                        string xmlPath = "";

                                        foreach (string s in elements)
                                        {
                                            xmlPath = s + "/" + xmlPath;
                                        }

                                        nvpair.Name = xmlPath + nvpair.Name;

                                        configNameValuePairs.Add(nvpair);
                                    }
                                }
                            }

                            if (isEmptyElement && elements.Count != 0)
                                elements.Pop();

                            break;
                        case XmlNodeType.Text:
                            // no need to support text yet
                            break;
                        case XmlNodeType.EndElement:

                            string peek = elements.Peek();

                            if (String.Compare(peek, xmlReader.Name, true) == 0)
                            {
                                elements.Pop();
                            }

                            break;
                    }
                }
            }
            catch //(Exception ex)
            {
                //MessageBox.Show(ex.ToString());
            }

            return true;
        }

        public static void Close()
        {
            if (xmlReader == null)
                return;

            xmlReader.Close();
            xmlReader = null;
            
            ms.Close();
            ms = null;
        }

        public static void ClearConfig()
        {
            configNameValuePairs.Clear();
        }

        public static string GetConfigString(string xmlPath)
        {
            foreach (NameValuePair nvpair in configNameValuePairs)
            {
                if (String.Compare(nvpair.Name, xmlPath, true) == 0)
                {
                    return nvpair.Value;
                }
            }

            return null;
        }

        public static bool GetConfigBool(string xmlPath)
        {
            string stringBool = GetConfigString(xmlPath);

            if (stringBool != null && String.Compare(stringBool, "true", true) == 0)
            {
                return true;
            }

            return false;
        }

        public static string[] GetConfigStringArray(string xmlPath)
        {
            string stringArray = GetConfigString(xmlPath);
            
            if (stringArray != null)
            {
                return stringArray.Split('|');
            }

            return null;
        }

        public static NameValuePair[] GetNameValuePairArray(string partialXmlPath)
        {
            List<NameValuePair> listNVPair = new List<NameValuePair>();

            foreach (NameValuePair nvpair in configNameValuePairs)
            {
                if (String.Compare(partialXmlPath, 0, nvpair.Name, 0, partialXmlPath.Length, true) == 0)
                {
                    listNVPair.Add(nvpair);
                }
            }

            return (listNVPair.Count == 0 ? null : listNVPair.ToArray());
        }

        public static string GetNameFromPath(string xmlPath)
        {
            int idx = 0;

            if (xmlPath != null && (idx = xmlPath.LastIndexOf('/')) != -1)
            {
                return xmlPath.Substring(idx + 1);
            }

            return null;
        }

        public static bool SetConfigString(string xmlPath, string value)
        {
            NameValuePair nvpEntry = null;

            foreach (NameValuePair nvpair in configNameValuePairs)
            {
                if (String.Compare(nvpair.Name, xmlPath, true) == 0)
                {
                    nvpEntry = nvpair;
                    break;
                }
            }

            if (nvpEntry == null)
                return false;

            try
            {
                NaiveConfigWriter writer = new NaiveConfigWriter(configFilePath);

                if (!writer.SetExistingPathValue(xmlPath, value))
                    return false;

                // update our config setting only if writer was successful
                nvpEntry.Value = value;
            }
            catch
            {
                return false;
            }

            return true;
        }

        private static string configFilePath = null;
        private static MemoryStream ms = null;
        private static XmlTextReader xmlReader = null;

        public class NameValuePair
        {
            public string Name = null;
            public string Value = null;

            public bool IsValid()
            {
                return (Name != null && Value != null);
            }
        }

        private static List<NameValuePair> configNameValuePairs = new List<NameValuePair>();
    }

    public class NaiveConfigWriter
    {
        public NaiveConfigWriter(string path)
        {
            xmlFilePath = path;
        }

        public bool SetExistingPathValue(string xmlPath, string value)
        {
            try
            {
                string[] pathComponents = xmlPath.Split('/');
                
                string xmlText = File.ReadAllText(xmlFilePath);

                if (String.IsNullOrEmpty(xmlText))
                    return false;

                using (MemoryStream ms = new MemoryStream(Encoding.ASCII.GetBytes(xmlText)))
                {
                    using (XmlTextReader xmlReader = new XmlTextReader(ms))
                    {
                        using (XmlTextWriter xmlWriter = new XmlTextWriter(xmlFilePath, Encoding.UTF8))
                        {
                            xmlWriter.Formatting = Formatting.Indented;
                            
                            int depth = 0;

                            xmlWriter.WriteStartDocument();

                            while (xmlReader.Read())
                            {
                                switch (xmlReader.NodeType)
                                {
                                    case XmlNodeType.Element:

                                        bool isEmptyElement = xmlReader.IsEmptyElement, shouldPop = false;

                                        xmlWriter.WriteStartElement(xmlReader.Name);

                                        if (String.Compare(xmlReader.Name, pathComponents[depth], true) == 0)
                                        {
                                            shouldPop = isEmptyElement;
                                            depth++;
                                        }

                                        if (xmlReader.HasAttributes)
                                        {
                                            if (xmlReader.MoveToFirstAttribute())
                                            {
                                                string name = null;
                                                string val = null;

                                                do
                                                {
                                                    if (xmlReader.Name == "name")
                                                        name = xmlReader.Value;

                                                    if (xmlReader.Name == "value")
                                                        val = xmlReader.Value;

                                                    if (name != null &&
                                                        val != null &&
                                                        depth + 1 == pathComponents.Length &&
                                                        String.Compare(name, pathComponents[depth], true) == 0)
                                                    {
                                                        xmlWriter.WriteAttributeString(xmlReader.Name, value);
                                                    }
                                                    else
                                                    {
                                                        xmlWriter.WriteAttributeString(xmlReader.Name, xmlReader.Value);
                                                    }
                                                }
                                                while (xmlReader.MoveToNextAttribute());
                                            }
                                        }

                                        if (isEmptyElement)
                                            xmlWriter.WriteEndElement();

                                        if (shouldPop)
                                            depth--;

                                        break;
                                    case XmlNodeType.Comment:
                                        xmlWriter.WriteComment(xmlReader.Value);

                                        break;
                                    case XmlNodeType.Text:
                                        // no need to support text yet
                                        break;
                                    case XmlNodeType.EndElement:

                                        if (depth > 0 && String.Compare(xmlReader.Name, pathComponents[depth - 1], true) == 0)
                                        {
                                            depth--;
                                        }

                                        xmlWriter.WriteEndElement();
                                        
                                        break;
                                }
                            }
                        }
                    }
                }
            }
            catch //(Exception ex)
            {
                //MessageBox.Show(ex.ToString());
            }

            return true;
        }

        public bool AddXmlPropertyAtPath(string xmlPath, string name, string value)
        {
            try
            {
                string[] xmlConfig = File.ReadAllLines(xmlFilePath);
                string[] elements = xmlPath.Split('/');

                StringBuilder sb = new StringBuilder(xmlConfig.Length + name.Length + value.Length + 128);
                int depth = 0;
                bool insertComplete = false;

                foreach (string line in xmlConfig)
                {
                    sb.Append(line + "\r\n");
                    if (insertComplete)
                        continue;

                    int idx = line.IndexOf("<" + elements[depth] + ">");
                    if (idx == -1)
                    {
                        if (depth > 0)
                        {
                            idx = line.IndexOf("</" + elements[depth - 1] + ">");
                            if (idx != -1)
                            {
                                depth--;
                            }
                        }

                        continue;
                    }

                    depth++;

                    if(depth < elements.Length - 1)
                        continue;

                    // insert here
                    int whiteSpace = 0;
                    
                    while (Char.IsWhiteSpace(line[whiteSpace]))
                        whiteSpace++;

                    sb.Append(new string(' ', whiteSpace + 2) + "<property name=\"" + name + "\" value=\"" + value + "\" />\r\n");
                    insertComplete = true;
                }

                string newConfig = sb.ToString();
                
                File.WriteAllText(xmlFilePath, newConfig);
            }
            catch
            {
                return false;
            }

            return true;
        }

        public bool RemoveXmlPropertyWithPath(string xmlPath)
        {
            try
            {
                string[] xmlConfig = File.ReadAllLines(xmlFilePath);
                string[] elements = xmlPath.Split('/');

                StringBuilder sb = new StringBuilder(xmlConfig.Length);
                int depth = 0;
                bool deleteComplete = false;

                foreach (string line in xmlConfig)
                {
                    sb.Append(line + "\r\n");

                    if (deleteComplete)
                        continue;

                    int idx = line.IndexOf((depth == elements.Length - 1) ? "property" : "<" + elements[depth]);
                    if (idx == -1)
                    {
                        if (depth > 0)
                        {
                            idx = line.IndexOf("</" + elements[depth - 1] + ">");
                            if (idx != -1)
                            {
                                depth--;
                            }
                        }

                        continue;
                    }

                    if (depth < elements.Length - 1)
                        depth++;

                    if (depth < elements.Length - 1)
                        continue;

                    // remove element

                    idx = line.IndexOf("name=\"" + elements[depth] + "\"");
                    if (idx == -1)
                        continue;

                    sb.Remove(sb.Length - line.Length - 2, line.Length + 2);
                    deleteComplete = true;
                }

                string newConfig = sb.ToString();

                File.WriteAllText(xmlFilePath, newConfig);
            }
            catch
            {
                return false;
            }

            return true;
        }

        private string xmlFilePath = null;
    }
}
