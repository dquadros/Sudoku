using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace SudokuBF
{
    class Sudoku
    {
        // Tamanho do diagrama
        private const int TGRID = 81;
        private const int TSIDE = 9;

        // Célula do diagrama
        // bits 15 a 13 não usados
        // bits 12 a 4  1 se digito 9 a 1 permitido
        // bits  3 a 0  digito ou 0 se ainda não conhecido
        private UInt16[] grid;  // diagrama
        private int unknow;     // número de células não conhecidas
        private int ls, cs;     // linha e coluna atuais
        private UInt16 dig;     // dígito na linha e coluna atuais

        // Construtor
        public Sudoku()
        {
            grid = new UInt16[TGRID];
            unknow = TGRID;
            ls = cs = 0;
        }

        // Testa se resolveu
        public Boolean Solved()
        {
            return unknow == 0;
        }

        // Verifica se posição conhecida
        public Boolean Found(int lin, int col)
        {
            int val = grid[lin * TSIDE + col] & 0xF;
            return val != 0;
        }

        // Procura a próxima ceélula ainda não determinada
        public void Next()
        {
            while (Found(ls, cs))
            {
                if (++cs == TSIDE)
                {
                    cs = 0;
                    ls++;
                }
            }
        }

        // Coloca o próximo valor na célula atual
        // Retorna false se j[á testou todas as opções
        public Boolean NextVal()
        {
            UInt16 mask;
            int val = grid[ls * TSIDE + cs];
            dig = (UInt16) (val & 0xF);
            while (dig < 9)
            {
                val++;
                dig++;
                mask = (UInt16) (1 << (dig+3));
                if ((val & mask) != 0)
                {
                    Mark (ls, cs, dig);
                    return true;
                }
            }
            return false;
        }

        // Coloca dígito em uma posição
        public void Mark(int lin, int col, UInt16 val)
        {
            //Console.Out.WriteLine("Mark ["+lin.ToString()+","+col.ToString()+
            //    "]="+val.ToString());
            UInt16 mask = (UInt16) (~(1 << (val+3)));
            grid[lin * TSIDE + col] = val;
            for (int l = 0; l < TSIDE; l++)
            {
                grid[l * TSIDE + col] &= mask;
            }
            for (int c = 0; c < TSIDE; c++)
            {
                grid[lin * TSIDE + c] &= mask;
            }
            lin -= lin %3;
            col -= col%3;
            for (int l = 0; l < 3; l++)
            {
                for (int c = 0; c < 3; c++)
                {
                    grid[(lin + l) * TSIDE + col + c] &= mask;
                }
            }
            unknow--;
        }

        // Carrega um problema
        // Retorna false se algo der errado
        public Boolean Load(string arq)
        {
            try
            {
                for (int lin = 0; lin < TSIDE; lin++)
                {
                    for (int col = 0; col < TSIDE; col++)
                    {
                        grid[lin * TSIDE + col] = 0x1FF0;
                    }
                }
                using (StreamReader sr = new StreamReader(arq))
                {
                    for (int lin = 0; lin < TSIDE; lin++)
                    {
                        string linha = sr.ReadLine();
                        for (int col = 0; col < TSIDE; col++)
                        {
                            if ((linha[col] >= '1') && (linha[col] <= '9'))
                            {
                                UInt16 val = (UInt16)(linha[col] - '0');
                                Mark(lin, col, val);
                            }
                        }
                    }
                }
                return true;
            }
            catch
            {
                return false;
            }
        }

        // Imprime o diagrama
        public void Print()
        {
            for (int lin = 0; lin < TSIDE; lin++)
            {
                for (int col = 0; col < TSIDE; col++)
                {
                    int val = grid[lin * TSIDE + col] & 0xF;
                    if (val == 0)
                    {
                        Console.Out.Write('.');
                    }
                    else
                    {
                        Console.Out.Write(val);
                    }
                    Console.Out.Write(' ');
                }
                Console.Out.WriteLine();
            }
            Console.Out.WriteLine();
        }

        // Copia o objeto
        public Sudoku Copy()
        {
            Sudoku s = new Sudoku();
            s.unknow = unknow;
            for (int i = 0; i < TGRID; i++)
            {
                s.grid[i] = grid[i];
            }
            return s;
        }

        // Resolve o problema
        public Boolean Solve()
        {
            while (!Solved())
            {
                Sudoku s = Copy();
                s.Next();
                if (s.NextVal())
                {
                    if (s.Solve())
                    {
                        return true;
                    }
                    else
                    {
                        grid[s.ls * TSIDE + s.cs] &= (UInt16)(~(1 << (s.dig + 3)));
                    }
                }
                else
                {
                    return false;
                }
            }
            Print();
            return true;
        }
    }
}
