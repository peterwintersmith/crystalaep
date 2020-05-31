// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.IO.Pipes;

namespace Logger
{
    class Logger
    {
        public Logger(string pipeName)
        {
            clientPipeName = pipeName;
        }

        public void Start()
        {
            while (true)
            {
                try
                {
                    using (NamedPipeServerStream serverPipe = new NamedPipeServerStream(clientPipeName))
                    {
                        Console.WriteLine(new String('-', Console.WindowWidth));
                        Console.WriteLine("Waiting for client connection ...");
                        serverPipe.WaitForConnection();
                        Console.WriteLine("Client connected at " + DateTime.Now);

                        try
                        {
                            using (StreamReader sr = new StreamReader(serverPipe))
                            {
                                while (true)
                                {
                                    string lineRead = sr.ReadLine();
                                    if (lineRead.Equals("quit"))
                                        break;

                                    Console.WriteLine(lineRead);
                                }
                            }
                        }
                        catch (Exception ex)
                        {
                            //Console.WriteLine("An exception has occurred [{0}] ({1}).\n", ex.GetType().Name, ex.Message);
                        }

                        Console.WriteLine("Connection closed.");
                    }
                }
                catch(ObjectDisposedException)
                {
                    // eat exception
                }
            }
        }
        
        private string clientPipeName = null;
    }

    class Program
    {
        static void Main(string[] args)
        {
            string appName = Environment.CommandLine.Substring(0,
                Environment.CommandLine.IndexOf(' ') == -1 ?
                Environment.CommandLine.Length :
                Environment.CommandLine.IndexOf(' ')
                );

            Console.WriteLine("BP Logger Util");

            if (args.Length == 0)
            {
                Console.WriteLine("Usage: {0} <pipe name>", appName);
                return;
            }

            try
            {
                Logger logger = new Logger(args[0]);
                logger.Start();
                Console.WriteLine("Logger exiting.");
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: {0} [{1}] [\n{2}\n]", ex.Message, ex.GetType().Name, ex.StackTrace);
            }
        }
    }
}
