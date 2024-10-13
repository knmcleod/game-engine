using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using GE;

namespace demo
{
    public class Camera : Entity
    {
        public Entity FollowEntity;

        public float FollowDistance = 1.0f;
        public float HeightOffset = 1.0f;
        public float WidthOffset = 1.0f;
        protected override void OnCreate()
        {
            FollowEntity = FindEntityByName("Player");
            Translation = (FollowEntity != null
                ? new Vector3(FollowEntity.Translation.x + WidthOffset, FollowEntity.Translation.y + HeightOffset, FollowDistance)
                : new Vector3(WidthOffset, HeightOffset, FollowDistance));
        }

        protected override void OnUpdate(float timestep)
        {
            if (FollowEntity != null)
            {
                Translation = new Vector3(FollowEntity.Translation.x + WidthOffset,
                    FollowEntity.Translation.y + HeightOffset, FollowDistance);
            }
            else
            {
                FollowEntity = FindEntityByName("Player");
            }
        }
    }
}