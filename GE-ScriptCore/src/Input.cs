﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GE
{
    public class Input
    {
        public static bool IsKeyDown(KeyCode keyCode)
        {
            return InternalCalls.Input_IsKeyDown(keyCode);
        }

        public static bool IsMouseDown(MouseCode mouseCode) 
        {
            return InternalCalls.Input_IsMouseDown(mouseCode);
        }
    }
}
