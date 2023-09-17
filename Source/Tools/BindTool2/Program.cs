using System;
using System.IO;
using System.Threading;
using CppAst;

static class Program
{
    static void Main(string[] args)
    {
        string sourceFile = @"C:\Home\source\rbfx\rbfx\Source\Urho3D\BindAll.cpp";
        string flagsFile = @"C:\Home\source\rbfx\rbfx\cmake-build\Windows-Desktop-Debug\Source\Urho3D\GeneratorOptions_Urho3D_RelWithDebInfo.txt";

        CppParserOptions options = new CppParserOptions();
        options.AdditionalArguments.Add("-std=c++17");
        using (var sr = new StreamReader(flagsFile))
        {
            while (!sr.EndOfStream)
                options.AdditionalArguments.Add(sr.ReadLine());
        }

        foreach (string file in Directory.EnumerateFiles(@"C:\Home\source\rbfx\rbfx\Source\Urho3D", "*.h*", SearchOption.AllDirectories))
        {
            if (file.EndsWith(".h") || file.EndsWith(".hpp"))
            {
                if (file.EndsWith("BindAll.cpp") || file.EndsWith("Precompiled.h"))
                    continue;
                CppCompilation ast = CppParser.ParseFile(file, options);
                if (ast.HasErrors)
                {
                    foreach (var msg in ast.Diagnostics.Messages)
                    {
                        if (msg.Type == CppLogMessageType.Error)
                        {
                            Console.WriteLine($"{msg.Location}: {msg.Text}");
                        }
                    }
                }
                ast = null;
            }
        }
    }
}
