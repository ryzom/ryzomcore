scenario = {
  InstanceId = [[Client1_27]],
  Class = [[Scenario]],
  AccessRules = [[liberal]],
  InheritPos = 1,
  Name = [[Unknown class]],
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
              Class = [[Npc]],
              Angle = 0.5,
              Base = [[palette.entities.botobjects.campfire]],
              InheritPos = 1,
              Name = [[camp fire 1]],
              ActivitiesId = {
              },
              Behavior = {
                InstanceId = [[Client1_3]],
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
                InstanceId = [[Client1_4]],
                Class = [[Position]],
                x = 24370,
                y = -1363,
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
                  {
                    InstanceId = [[Client1_9]],
                    Class = [[LogicEntityAction]],
                    Name = [[]],
                    Actions = {
                      {
                        InstanceId = [[Client1_5]],
                        Class = [[ActionStep]],
                        Entity = r2.RefId([[Client1_870]]),
                        Action = {
                          InstanceId = [[Client1_6]],
                          Class = [[ActionType]],
                          Type = [[activate]],
                          Value = r2.RefId([[]])
                        }
                      },
                      {
                        InstanceId = [[Client1_7]],
                        Class = [[ActionStep]],
                        Entity = r2.RefId([[Client1_871]]),
                        Action = {
                          InstanceId = [[Client1_8]],
                          Class = [[ActionType]],
                          Type = [[Kill]],
                          Value = r2.RefId([[]])
                        }
                      }
                    },
                    Conditions = {
                    },
                    Event = {
                      InstanceId = [[Client1_10]],
                      Class = [[EventType]],
                      Type = [[death]],
                      Value = r2.RefId([[]])
                    }
                  },
                  {
                    InstanceId = [[Client1_15]],
                    Class = [[LogicEntityAction]],
                    Name = [[]],
                    Actions = {
                      {
                        InstanceId = [[Client1_11]],
                        Class = [[ActionStep]],
                        Entity = r2.RefId([[Client1_871]]),
                        Action = {
                          InstanceId = [[Client1_12]],
                          Class = [[ActionType]],
                          Type = [[Kill]],
                          Value = r2.RefId([[]])
                        }
                      }
                    },
                    Conditions = {
                      {
                        InstanceId = [[Client1_13]],
                        Class = [[ConditionStep]],
                        Entity = r2.RefId([[Client1_870]]),
                        Condition = {
                          InstanceId = [[Client1_14]],
                          Class = [[ConditionType]],
                          Type = [[is active]],
                          Value = r2.RefId([[]])
                        }
                      }
                    },
                    Event = {
                      InstanceId = [[Client1_16]],
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
                InstanceId = [[Client1_18]],
                Class = [[Position]],
                x = 24380,
                y = -1370,
                z = 0
              }
            }
          }
        },
        {
          InstanceId = [[Client1_870]],
          Class = [[MadeUpFeature]],
          InheritPos = 1,
          Name = [[Gadget 1]],
          Behavior = {
            InstanceId = [[Client1_1]],
            Class = [[LogicEntityBehavior]],
            Actions = {
            }
          },
          Components = {
          },
          Position = {
            InstanceId = [[Client1_2]],
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
    Title = [[Unknown class]]
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
    ConditionStep = 0,
    ConditionType = 0,
    DefaultFeature = 0,
    EventType = 0,
    Location = 1,
    LogicEntityAction = 0,
    LogicEntityBehavior = 1,
    MadeUpFeature = 1,
    MapDescription = 0,
    Npc = 1,
    NpcCreature = 1,
    Position = 0,
    Scenario = 4,
    TextManager = 0
  }
}
