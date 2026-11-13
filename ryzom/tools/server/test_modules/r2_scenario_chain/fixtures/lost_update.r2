scenario = {
  InstanceId = [[Client1_15]],
  Class = [[Scenario]],
  AccessRules = [[liberal]],
  InheritPos = 1,
  Name = [[Lost update]],
  VersionName = [[fixture]],
  Acts = {
    {
      InstanceId = [[Client1_2]],
      Class = [[Act]],
      Version = 6,
      InheritPos = 1,
      LocationId = [[]],
      Season = 0,
      ShortDescription = [[]],
      Title = [[Permanent]],
      ActivitiesIds = {
      },
      Counters = {
      },
      Events = {
      },
      Features = {
        {
          InstanceId = [[Client1_1]],
          Class = [[DefaultFeature]],
          Components = {
          }
        }
      },
      Position = {
        InstanceId = [[Client1_4]],
        Class = [[Position]],
        x = 0,
        y = 0,
        z = 0
      }
    },
    {
      InstanceId = [[Client1_12]],
      Class = [[Act]],
      Version = 6,
      LocationId = [[Client1_900]],
      Season = 0,
      ShortDescription = [[]],
      Title = [[Act 1]],
      ActivitiesIds = {
      },
      Counters = {
      },
      Events = {
      },
      Features = {
        {
          InstanceId = [[Client1_11]],
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
                x = 24370,
                y = -1363,
                z = 0
              }
            },
            {
              InstanceId = [[Client1_8]],
              Class = [[NpcCreature]],
              Angle = 0.5,
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
                x = 24380,
                y = -1370,
                z = 0
              }
            }
          }
        }
      },
      Position = {
        InstanceId = [[Client1_14]],
        Class = [[Position]],
        x = 0,
        y = 0,
        z = 0
      }
    }
  },
  Description = {
    InstanceId = [[Client1_17]],
    Class = [[MapDescription]],
    LevelId = 0,
    OptimalNumberOfPlayer = 0,
    ShortDescription = [[]],
    Title = [[Lost update]]
  },
  Locations = {
    {
      InstanceId = [[Client1_900]],
      Class = [[Location]],
      EntryPoint = [[uiR2EntryPoint01]],
      IslandName = [[uiR2_Deserts05]],
      ManualSeason = 1,
      Name = [[The dunes]],
      Season = [[Spring]],
      ShortDescription = [[]],
      Time = 0
    }
  },
  Position = {
    InstanceId = [[Client1_18]],
    Class = [[Position]],
    x = 0,
    y = 0,
    z = 0
  },
  Texts = {
    InstanceId = [[Client1_19]],
    Class = [[TextManager]],
    Texts = {
    }
  },
  Versions = {
    Act = 6,
    Behavior = 1,
    DefaultFeature = 0,
    Location = 1,
    MapDescription = 0,
    Npc = 1,
    NpcCreature = 1,
    Position = 0,
    Scenario = 4,
    TextManager = 0
  }
}
