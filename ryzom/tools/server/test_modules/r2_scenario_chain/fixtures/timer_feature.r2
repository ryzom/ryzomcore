scenario = {
  InstanceId = [[Client1_27]],
  Class = [[Scenario]],
  AccessRules = [[liberal]],
  InheritPos = 1,
  Name = [[Timer feature]],
  VersionName = [[fixture]],
  Acts = {
    {
      InstanceId = [[Client1_20]],
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
        InstanceId = [[Client1_21]],
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
          InstanceId = [[Client1_19]],
          Class = [[DefaultFeature]],
          Components = {
          }
        }
      },
      Position = {
        InstanceId = [[Client1_22]],
        Class = [[Position]],
        x = 0,
        y = 0,
        z = 0
      }
    },
    {
      InstanceId = [[Client1_24]],
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
        InstanceId = [[Client1_25]],
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
          InstanceId = [[Client1_23]],
          Class = [[DefaultFeature]],
          Components = {
            {
              InstanceId = [[Client1_871]],
              Class = [[NpcCreature]],
              Angle = 0.5,
              Base = [[palette.entities.creatures.ckdff1]],
              InheritPos = 1,
              Name = [[Gruesome Kincher]],
              ActivitiesId = {
              },
              Behavior = {
                InstanceId = [[Client1_15]],
                Class = [[Behavior]],
                Type = [[]],
                ZoneId = [[]],
                Actions = {
                  {
                    InstanceId = [[Client1_13]],
                    Class = [[LogicEntityAction]],
                    Name = [[]],
                    Actions = {
                      {
                        InstanceId = [[Client1_7]],
                        Class = [[ActionStep]],
                        Entity = r2.RefId([[Client1_870]]),
                        Action = {
                          InstanceId = [[Client1_8]],
                          Class = [[ActionType]],
                          Type = [[Desactivate]],
                          Value = r2.RefId([[]])
                        }
                      },
                      {
                        InstanceId = [[Client1_9]],
                        Class = [[ActionStep]],
                        Entity = r2.RefId([[Client1_870]]),
                        Action = {
                          InstanceId = [[Client1_10]],
                          Class = [[ActionType]],
                          Type = [[Sub 1 min]],
                          Value = r2.RefId([[]])
                        }
                      },
                      {
                        InstanceId = [[Client1_11]],
                        Class = [[ActionStep]],
                        Entity = r2.RefId([[Client1_870]]),
                        Action = {
                          InstanceId = [[Client1_12]],
                          Class = [[ActionType]],
                          Type = [[Pause]],
                          Value = r2.RefId([[]])
                        }
                      }
                    },
                    Conditions = {
                    },
                    Event = {
                      InstanceId = [[Client1_14]],
                      Class = [[EventType]],
                      Type = [[death]],
                      Value = r2.RefId([[]])
                    }
                  }
                },
                Activities = {
                },
                ChatSequences = {
                }
              },
              Position = {
                InstanceId = [[Client1_16]],
                Class = [[Position]],
                x = 24380,
                y = -1370,
                z = 0
              }
            },
            {
              InstanceId = [[Client1_872]],
              Class = [[NpcCreature]],
              Angle = 0.5,
              Base = [[palette.entities.creatures.ckdff1]],
              InheritPos = 1,
              Name = [[Gruesome Kincher]],
              ActivitiesId = {
              },
              Behavior = {
                InstanceId = [[Client1_17]],
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
                InstanceId = [[Client1_18]],
                Class = [[Position]],
                x = 24384,
                y = -1372,
                z = 0
              }
            }
          }
        },
        {
          InstanceId = [[Client1_870]],
          Class = [[TimerFeature]],
          Base = [[palette.entities.botobjects.campfire]],
          Cyclic = 0,
          InheritPos = 1,
          Minutes = 0,
          Name = [[Timer 1]],
          Secondes = 40,
          Behavior = {
            InstanceId = [[Client1_5]],
            Class = [[Behavior]],
            Type = [[]],
            ZoneId = [[]],
            Actions = {
              {
                InstanceId = [[Client1_3]],
                Class = [[LogicEntityAction]],
                Name = [[]],
                Actions = {
                  {
                    InstanceId = [[Client1_1]],
                    Class = [[ActionStep]],
                    Entity = r2.RefId([[Client1_872]]),
                    Action = {
                      InstanceId = [[Client1_2]],
                      Class = [[ActionType]],
                      Type = [[Kill]],
                      Value = r2.RefId([[]])
                    }
                  }
                },
                Conditions = {
                },
                Event = {
                  InstanceId = [[Client1_4]],
                  Class = [[EventType]],
                  Type = [[On Trigger]],
                  Value = r2.RefId([[]])
                }
              }
            },
            Activities = {
            },
            ChatSequences = {
            }
          },
          Components = {
          },
          Position = {
            InstanceId = [[Client1_6]],
            Class = [[Position]],
            x = 24375,
            y = -1365,
            z = 0
          }
        }
      },
      Position = {
        InstanceId = [[Client1_26]],
        Class = [[Position]],
        x = 0,
        y = 0,
        z = 0
      }
    }
  },
  Behavior = {
    InstanceId = [[Client1_28]],
    Class = [[LogicEntityBehavior]],
    Actions = {
    }
  },
  Description = {
    InstanceId = [[Client1_29]],
    Class = [[MapDescription]],
    LevelId = 0,
    OptimalNumberOfPlayer = 0,
    ShortDescription = [[]],
    Title = [[Timer feature]]
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
    InstanceId = [[Client1_30]],
    Class = [[Position]],
    x = 0,
    y = 0,
    z = 0
  },
  Texts = {
    InstanceId = [[Client1_31]],
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
    EventType = 0,
    Location = 1,
    LogicEntityAction = 0,
    LogicEntityBehavior = 1,
    MapDescription = 0,
    NpcCreature = 1,
    Position = 0,
    Scenario = 4,
    TextManager = 0,
    TimerFeature = 0
  }
}
