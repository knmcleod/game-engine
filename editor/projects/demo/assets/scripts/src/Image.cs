using GE;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace demo
{
    public class Image : Entity
    {
        private GUIImageComponent imageComponent;

        protected override void OnCreate()
        {
            imageComponent = GetComponent<GUIImageComponent>();
        }

        protected override void OnUpdate(float ts)
        {
            if (imageComponent == null)
                imageComponent = GetComponent<GUIImageComponent>();
        }
        protected override bool OnEvent(Event e)
        {
            return base.OnEvent(e);
        }

        protected override bool OnMousePressed(Event e, MouseCode mouseCode)
        {
            Log.LogCoreInfo("Image.OnMousePressed()");
            return false;
        }

        protected override bool OnKeyPressed(Event e, KeyCode keyCode)
        {
            Log.LogCoreInfo("Image.OnKeyPressed()");
            return false;
        }
    }
}
