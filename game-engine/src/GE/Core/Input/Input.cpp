#include "GE/GEpch.h"

#include "Input.h"

#include "../Application/Application.h"

namespace GE
{
    std::string Input::GetStringFromMouseButton(const Input::MouseCode& button)
    {
        switch (button)
        {
        case Input::MOUSE_BUTTON_1:
            return "Mouse1";
            break;

        case Input::MOUSE_BUTTON_2:
            return "Mouse2";
            break;

        case Input::MOUSE_BUTTON_3:
            return "Mouse3";
            break;

        case Input::MOUSE_BUTTON_4:
            return "Mouse4";
            break;

        case Input::MOUSE_BUTTON_5:
            return "Mouse5";
            break;

        case Input::MOUSE_BUTTON_6:
            return "Mouse6";
            break;

        case Input::MOUSE_BUTTON_7:
            return "Mouse7";
            break;

        case Input::MOUSE_BUTTON_8:
            return "Mouse8";
            break;

        default:
            GE_CORE_ERROR("Unrecognized mouse key");
            break;
        }
        return "None";
    }
    
    const char Input::GetCharFromKeyCode(const Input::KeyCode& keycode)
    {
        // TODO : Add caps lock button
        bool capsLock = Application::IsKeyPressed(Input::KEY_LEFT_SHIFT) || Application::IsKeyPressed(Input::KEY_RIGHT_SHIFT);
        switch (keycode)
        {
        case GE::Input::KEY_NONE:
            break;
        case GE::Input::KEY_SPACE:
            return ' ';
            break;
        case GE::Input::KEY_0:
            return '0';
            break;
        case GE::Input::KEY_1:
            return '1';
            break;
        case GE::Input::KEY_2:
            return '2';
            break;
        case GE::Input::KEY_3:
            return '3';
            break;
        case GE::Input::KEY_4:
            return '4';
            break;
        case GE::Input::KEY_5:
            return '5';
            break;
        case GE::Input::KEY_6:
            return '6';
            break;
        case GE::Input::KEY_7:
            return '7';
            break;
        case GE::Input::KEY_8:
            return '8';
            break;
        case GE::Input::KEY_9:
            return '9';
            break;
        case GE::Input::KEY_A:
            if (capsLock)
                return 'A';
            return 'a';
            break;
        case GE::Input::KEY_B:
            if (capsLock)
                return 'B';
            return 'b';
            break;
        case GE::Input::KEY_C:
            if (capsLock)
                return 'C';
            return 'c';
            break;
        case GE::Input::KEY_D:
            if (capsLock)
                return 'D';
            return 'd';
            break;
        case GE::Input::KEY_E:
            if (capsLock)
                return 'E';
            return 'e';
            break;
        case GE::Input::KEY_F:
            if (capsLock)
                return 'F';
            return 'f';
            break;
        case GE::Input::KEY_G:
            if (capsLock)
                return 'G';
            return 'g';
            break;
        case GE::Input::KEY_H:
            if (capsLock)
                return 'H';
            return 'h';
            break;
        case GE::Input::KEY_I:
            if (capsLock)
                return 'I';
            return 'i';
            break;
        case GE::Input::KEY_J:
            if (capsLock)
                return 'J';
            return 'j';
            break;
        case GE::Input::KEY_K:
            if (capsLock)
                return 'K';
            return 'k';
            break;
        case GE::Input::KEY_L:
            if (capsLock)
                return 'L';
            return 'l';
            break;
        case GE::Input::KEY_M:
            if (capsLock)
                return 'M';
            return 'm';
            break;
        case GE::Input::KEY_N:
            if (capsLock)
                return 'N';
            return 'n';
            break;
        case GE::Input::KEY_O:
            if (capsLock)
                return 'O';
            return 'o';
            break;
        case GE::Input::KEY_P:
            if (capsLock)
                return 'P';
            return 'p';
            break;
        case GE::Input::KEY_Q:
            if (capsLock)
                return 'Q';
            return 'q';
            break;
        case GE::Input::KEY_R:
            if (capsLock)
                return 'R';
            return 'r';
            break;
        case GE::Input::KEY_S:
            if (capsLock)
                return 'S';
            return 's';
            break;
        case GE::Input::KEY_T:
            if (capsLock)
                return 'T';
            return 't';
            break;
        case GE::Input::KEY_U:
            if (capsLock)
                return 'U';
            return 'u';
            break;
        case GE::Input::KEY_V:
            if (capsLock)
                return 'V';
            return 'v';
            break;
        case GE::Input::KEY_W:
            if (capsLock)
                return 'W';
            return 'w';
            break;
        case GE::Input::KEY_X:
            if (capsLock)
                return 'X';
            return 'x';
            break;
        case GE::Input::KEY_Y:
            if (capsLock)
                return 'Y';
            return 'y';
            break;
        case GE::Input::KEY_Z:
            if (capsLock)
                return 'Z';
            return 'z';
            break;
        default:
            break;
        }

        return '\0';
    }
}