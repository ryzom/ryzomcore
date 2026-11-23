scenario = {
  InstanceId = [[Client1_1]],
  Class = [[Scenario]],
  Acts = {
    {
      InstanceId = [[Client1_3]],
      Class = [[Act]],
      Title = [[The clearing]],
      ActivitiesIds = {
      },
      Counters = {
      },
      Events = {
      },
      Features = {
        {
          InstanceId = [[Client1_4]],
          Class = [[DefaultFeature]],
          Components = {
            {
              InstanceId = [[Client1_5]],
              Class = [[Npc]],
              Angle = 0.5,
              Base = [[palette.entities.botobjects.campfire]],
              InheritPos = 1,
              Name = [[camp fire 1]],
              ActivitiesId = {
              },
              Behavior = {
                InstanceId = [[Client1_6]],
                Class = [[Behavior]],
                Type = [[]],
                ZoneId = [[]],
                Actions = {
                },
                Activities = {
                },
                ChatSequences = {
                }
              },
              Position = {
                InstanceId = [[Client1_7]],
                Class = [[Position]],
                x = 26000,
                y = -1300,
                z = 0
              }
            },
            {
              InstanceId = [[Client1_8]],
              Class = [[NpcCreature]],
              Angle = 1,
              Base = [[palette.entities.creatures.ckdff1]],
              InheritPos = 1,
              Name = [[Gruesome Kincher]],
              ActivitiesId = {
              },
              Behavior = {
                InstanceId = [[Client1_9]],
                Class = [[Behavior]],
                Type = [[]],
                ZoneId = [[]],
                Actions = {
                },
                Activities = {
                },
                ChatSequences = {
                }
              },
              Position = {
                InstanceId = [[Client1_10]],
                Class = [[Position]],
                x = 26010,
                y = -1310,
                z = 0
              }
            }
          }
        },
        {
          InstanceId = [[Client1_11]],
          Class = [[NpcGrpFeature]],
          InheritPos = 1,
          Name = [[Group 1]],
          Components = {
          },
          Position = {
            InstanceId = [[Client1_12]],
            Class = [[Position]],
            x = 0,
            y = 0,
            z = 0
          }
        }
      }
    }
  },
  Description = {
    InstanceId = [[Client1_2]],
    Class = [[MapDescription]],
    LocationId = 2,
    ShortDescription = [[A camp fire and a kincher.]],
    Title = [[Island from positions]]
  }
}
