using System;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;

namespace csharp
{
    class Program
    {
        static bool debug = false;
        static void Main(string[] args)
        {
            string debugEnvVar = Environment.GetEnvironmentVariable("EXE_SHIM_DEBUG");
            if(debugEnvVar == "1") {
                debug = true;
            }

            // scan command line to exclude executable
            var result = ParseArgument(Environment.CommandLine, 0);
            string selfBin = result.arg;
            string rawArguments = Environment.CommandLine.Substring(result.endPosition);
            Debug(selfBin);
            Debug(rawArguments);

            // Parse configuration
            string configPath = Regex.Replace(selfBin, "\\.[^\\.]*$", ".config");
            Debug(configPath);
            string config = File.ReadAllText(configPath, Encoding.UTF8);
            string[] configLines = config.Split("\n");
            string targetExe = "";
            string argumentsPrefix = "";
            foreach(var _configLine in configLines) {
                var configLine = Regex.Replace(_configLine, "[\r\n]", "");
                int equalSignIndex = configLine.IndexOf('=');
                if(equalSignIndex < 1) continue;
                string key = configLine.Substring(0, equalSignIndex);
                string value = configLine.Substring(equalSignIndex + 1);
                switch(key) {
                    case "target":
                        targetExe = value;
                        break;
                    case "argumentsPrefix":
                        argumentsPrefix = value;
                        break;
                }
            }

            Debug("targetExe = " + targetExe);
            Debug("argumentsPrefix = " + argumentsPrefix);

            Process whereProcess = new Process();
            whereProcess.StartInfo.FileName = "C:\\Windows\\System32\\where.exe";
            whereProcess.StartInfo.Arguments = encodeArgument(targetExe);
            whereProcess.StartInfo.UseShellExecute = false;
            whereProcess.StartInfo.CreateNoWindow = true;
            whereProcess.StartInfo.RedirectStandardInput = true;
            whereProcess.StartInfo.RedirectStandardOutput = true;
            whereProcess.StartInfo.RedirectStandardError = true;
            // string output = "";
            // string error = "";
            // whereProcess.OutputDataReceived += (sender, _args) => output += _args.Data + "\n";
            // whereProcess.ErrorDataReceived += (sender, _args) => error += _args.Data + "\n";
            whereProcess.Start();
            // whereProcess.BeginOutputReadLine();
            // whereProcess.BeginErrorReadLine();
            var otask = whereProcess.StandardOutput.ReadToEndAsync();
            var etask = whereProcess.StandardError.ReadToEndAsync();
            whereProcess.WaitForExit();
            otask.Wait();
            etask.Wait();
            string output = otask.Result;
            string error = etask.Result;
            if(whereProcess.ExitCode != 0) {
                Debug(error);
                Debug("where.exe returned non-zero exit code: " + whereProcess.ExitCode);
                Environment.Exit(whereProcess.ExitCode);
            }

            Debug(output);

            var endOfFirstLine = output.IndexOf("\n");
            var targetExeFullPath = endOfFirstLine >= 0 ? output.Substring(0, endOfFirstLine) : output;
            Debug("targetExeFullPath = " + output);

            Process process = new Process();
            process.StartInfo.FileName = targetExeFullPath;
            // process.StartInfo.Arguments = Environment.CommandLine;
            process.StartInfo.Arguments = argumentsPrefix + rawArguments;
            process.StartInfo.UseShellExecute = false;
            process.Start();
            process.WaitForExit();
            Environment.Exit(process.ExitCode);
        }

        static string encodeArgument(string arg) {
            bool willBeWrapped = Regex.IsMatch(arg, " \t\n\v\"");
            // Escape double-quotes and preceding backslashes
            arg = Regex.Replace(arg, "(\\\\*)\"", "$1$1\\\"");
            // If necessary, escape trailing backslashes
            if(willBeWrapped) {
                arg = Regex.Replace(arg, "(\\\\+)$", "$1$1");
            }
            if(willBeWrapped) {
                return "\"" + arg + "\"";
            } else {
                return arg;
            }
        }

        static string encodeArguments(string[] args) {
            bool first = true;
            string acc = "";
            foreach(var arg in args) {
                if(!first) acc += " ";
                first = false;
                acc += encodeArgument(arg);
            }
            return acc;
        }

        public struct ParseArgumentResult {  
            public string arg;  
            public int endPosition;  
        }

        static ParseArgumentResult ParseArgument(string commandLine, int start) {
            bool inQuote = false;
            bool prevWasBackslash = false;
            string acc = "";
            char c = '\0';
            int i;
            for(i = start; i < commandLine.Length; i++) {
                c = commandLine[i];
                // Terminate at a space
                if(!inQuote && c == ' ') {
                    break;
                }
                if(c == '"') {
                    if(prevWasBackslash) {
                        acc += c;
                        prevWasBackslash = false;
                    } else {
                        inQuote = !inQuote;
                    }
                    continue;
                }
                if(c == '\\') {
                    if(prevWasBackslash) {
                        acc += c;
                        prevWasBackslash = false;
                        continue;
                    } else {
                        prevWasBackslash = true;
                        continue;
                    }
                }
                if(prevWasBackslash) {
                    acc += '\\';
                }
                prevWasBackslash = false;
                acc += c;
            }
            if(prevWasBackslash == true) {
                acc += c;
            }
            return new ParseArgumentResult {
                arg = acc,
                endPosition = i
            };
        }

        static void Debug(string message) {
            if(debug) {
                Console.WriteLine(message);
            }
        }
    }
}
