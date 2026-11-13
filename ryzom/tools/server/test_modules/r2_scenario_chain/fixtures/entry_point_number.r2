scenario = {
  InstanceId = [[Client1_15]],
  Class = [[Scenario]],
  AccessRules = [[liberal]],
  InheritPos = 1,
  Name = [[Entry point number]],
  VersionName = [[fixture]],
  Acts = {
    {
      InstanceId = [[Client1_8]],
      Class = [[Act]],
      Version = 6,
      InheritPos = 1,
      LocationId = [[]],
      ManualWeather = 0,
      Name = [[Permanent]],
      Season = 0,
      ShortDescription = [[]],
      Title = [[Permanent]],
      WeatherValue = 0,
      ActivitiesIds = {
      },
      Behavior = {
        InstanceId = [[Client1_9]],
        Class = [[LogicEntityBehavior]],
        Actions = {
        }
      },
      Counters = {
      },
      Events = {
      },
      Features = {
        {
          InstanceId = [[Client1_7]],
          Class = [[DefaultFeature]],
          Components = {
          }
        }
      },
      Position = {
        InstanceId = [[Client1_10]],
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
      InheritPos = 1,
      LocationId = [[Client1_900]],
      ManualWeather = 0,
      Name = [[Act 1]],
      Season = 0,
      ShortDescription = [[]],
      Title = [[Act 1]],
      WeatherValue = 0,
      ActivitiesIds = {
      },
      Behavior = {
        InstanceId = [[Client1_13]],
        Class = [[LogicEntityBehavior]],
        Actions = {
        }
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
              InstanceId = [[Client1_1]],
              Class = [[Npc]],
              Angle = 0.5,
              Base = [[palette.entities.botobjects.campfire]],
              InheritPos = 1,
              Name = [[camp fire 1]],
              ActivitiesId = {
              },
              Behavior = {
                InstanceId = [[Client1_2]],
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
                InstanceId = [[Client1_3]],
                Class = [[Position]],
                x = 27470,
                y = -2150,
                z = 0
              }
            },
            {
              InstanceId = [[Client1_4]],
              Class = [[NpcCreature]],
              Angle = 0.5,
              Base = [[palette.entities.creatures.ckdff1]],
              InheritPos = 1,
              Name = [[Gruesome Kincher]],
              ActivitiesId = {
              },
              Behavior = {
                InstanceId = [[Client1_5]],
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
                InstanceId = [[Client1_6]],
                Class = [[Position]],
                x = 27480,
                y = -2160,
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
  Behavior = {
    InstanceId = [[Client1_16]],
    Class = [[LogicEntityBehavior]],
    Actions = {
    }
  },
  Description = {
    InstanceId = [[Client1_17]],
    Class = [[MapDescription]],
    LevelId = 0,
    OptimalNumberOfPlayer = 0,
    ShortDescription = [[]],
    Title = [[Entry point number]]
  },
  Locations = {
    {
      InstanceId = [[Client1_900]],
      Class = [[Location]],
      EntryPoint = 0,
      IslandName = [[uiR2_Deserts23]],
      ManualSeason = 1,
      Name = [[The parallel ways]],
      Season = [[Spring]],
      ShortDescription = [[]],
      Time = 0
    },
    {
      InstanceId = [[Client1_901]],
      Class = [[Location]],
      EntryPoint = 1,
      IslandName = [[uiR2_Deserts22]],
      ManualSeason = 1,
      Name = [[The small plain]],
      Season = [[Spring]],
      ShortDescription = [[]],
      Time = 0
    }
  },
  PlotItems = {
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
  UserComponents = {
  },
  Versions = {
    Act = 3,
    Behavior = 1,
    DefaultFeature = 0,
    Location = 0,
    LogicEntityBehavior = 1,
    MapDescription = 0,
    Npc = 1,
    NpcCreature = 1,
    Position = 0,
    Scenario = 4,
    TextManager = 0
  }
}
