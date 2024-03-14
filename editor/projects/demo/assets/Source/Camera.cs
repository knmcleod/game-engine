using GE;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace demo
{
    public class Camera : Entity
    {
        public Entity FollowEntity;

        public float FollowDistance = 5.0f;
        void OnCreate()
        {
            Log.LogCoreInfo("Camera.OnCreate. Finding Player");
            FollowEntity = FindEntityByTag("Player");
            if(FollowEntity != null )
                Log.LogCoreInfo("Camera.OnCreate. Player Found. UUID = " + FollowEntity.ID);
            Translation = new Vector3(0);
        }

        void OnUpdate(float timestep)
        {
            if(FollowEntity != null)
            {
                Log.LogCoreInfo("Canera.OnUpdate. Follow Entity Found.");
                Translation = new Vector3(FollowEntity.Translation.x,
                    FollowEntity.Translation.y, FollowDistance);
            }
            else
            {
                Log.LogCoreInfo("Canera.OnUpdate. Finding Follow Entity");
                FollowEntity = FindEntityByTag("Player");
                if (FollowEntity != null)
                    Log.LogCoreInfo("Camera.OnCreate. Player Found. UUID = " + FollowEntity.ID);
            }
        }
    }
}
