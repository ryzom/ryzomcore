scenario = {
  InstanceId = [[Client1_17]],
  Class = [[Scenario]],
  AccessRules = [[liberal]],
  InheritPos = 1,
  Name = [[Event desactivation]],
  VersionName = [[fixture]],
  Acts = {
    {
      InstanceId = [[Client1_10]],
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
        InstanceId = [[Client1_11]],
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
          InstanceId = [[Client1_9]],
          Class = [[DefaultFeature]],
          Components = {
          }
        }
      },
      Position = {
        InstanceId = [[Client1_12]],
        Class = [[Position]],
        x = 0,
        y = 0,
        z = 0
      }
    },
    {
      InstanceId = [[Client1_14]],
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
        InstanceId = [[Client1_15]],
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
          InstanceId = [[Client1_13]],
          Class = [[DefaultFeature]],
          Components = {
            {
              InstanceId = [[Client1_800]],
              Class = [[Npc]],
              Angle = 0.5,
              Base = [[palette.entities.botobjects.campfire]],
              InheritPos = 1,
              Name = [[camp fire 1]],
              ActivitiesId = {
              },
              Behavior = {
                InstanceId = [[Client1_1]],
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
                InstanceId = [[Client1_2]],
                Class = [[Position]],
                x = 24370,
                y = -1363,
                z = 0
              }
            }
          }
        },
        {
          InstanceId = [[Client1_810]],
          Class = [[EasterEgg]],
          Active = 1,
          Base = [[palette.entities.botobjects.chest_wisdom_std_sel]],
          InheritPos = 1,
          Item1Id = r2.RefId([[]]),
          Item1Qty = 0,
          Item2Id = r2.RefId([[]]),
          Item2Qty = 0,
          Item3Id = r2.RefId([[]]),
          Item3Qty = 0,
          ItemNumber = 0,
          Name = [[Chest 1]],
          _Seed = 1154080571,
          Behavior = {
            InstanceId = [[Client1_7]],
            Class = [[LogicEntityBehavior]],
            Actions = {
              {
                InstanceId = [[Client1_5]],
                Class = [[LogicEntityAction]],
                Name = [[]],
                Actions = {
                  {
                    InstanceId = [[Client1_3]],
                    Class = [[ActionStep]],
                    Entity = r2.RefId([[Client1_810]]),
                    Action = {
                      InstanceId = [[Client1_4]],
                      Class = [[ActionType]],
                      Type = [[activate]],
                      Value = r2.RefId([[]])
                    }
                  }
                },
                Conditions = {
                },
                Event = {
                  InstanceId = [[Client1_6]],
                  Class = [[EventType]],
                  Type = [[desactivation]],
                  Value = r2.RefId([[]])
                }
              }
            }
          },
          Components = {
          },
          Position = {
            InstanceId = [[Client1_8]],
            Class = [[Position]],
            x = 24375,
            y = -1365,
            z = 0
          }
        }
      },
      Position = {
        InstanceId = [[Client1_16]],
        Class = [[Position]],
        x = 0,
        y = 0,
        z = 0
      }
    }
  },
  Behavior = {
    InstanceId = [[Client1_18]],
    Class = [[LogicEntityBehavior]],
    Actions = {
    }
  },
  Description = {
    InstanceId = [[Client1_19]],
    Class = [[MapDescription]],
    LevelId = 0,
    OptimalNumberOfPlayer = 0,
    ShortDescription = [[]],
    Title = [[Event desactivation]]
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
  PlotItems = {
  },
  Position = {
    InstanceId = [[Client1_20]],
    Class = [[Position]],
    x = 0,
    y = 0,
    z = 0
  },
  Texts = {
    InstanceId = [[Client1_21]],
    Class = [[TextManager]],
    Texts = {
    }
  },
  UserComponents = {
  },
  Versions = {
    Act = 6,
    ActionStep = 2,
    ActionType = 0,
    Behavior = 1,
    DefaultFeature = 0,
    EasterEgg = 1,
    EventType = 0,
    Location = 1,
    LogicEntityAction = 0,
    LogicEntityBehavior = 1,
    MapDescription = 0,
    Npc = 1,
    Position = 0,
    Scenario = 4,
    TextManager = 0
  }
}
