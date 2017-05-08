using System;
using System.Collections.Generic;
using System.Text;

namespace SudokuBF
{
    class Program
    {
        static void Main(string[] args)
        {
            Sudoku s = new Sudoku();
            if (s.Load("teste.txt"))
            {
                s.Print();
                DateTime inicio = DateTime.Now;
                s.Solve();
                TimeSpan ts = DateTime.Now - inicio;
                Console.Out.WriteLine("Tempo: " + ts.ToString());
            }
            else
            {
                Console.Out.WriteLine("Erro no arquivo");
            }
            System.Console.ReadKey();
        }
    }
}
