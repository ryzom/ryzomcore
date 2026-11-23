scenario = {
  InstanceId = [[Client1_31]],
  Class = [[Scenario]],
  AccessRules = [[liberal]],
  InheritPos = 1,
  Name = [[Healthy scenario]],
  VersionName = [[fixture]],
  Acts = {
    {
      InstanceId = [[Client1_24]],
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
    },
    {
      InstanceId = [[Client1_28]],
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
        InstanceId = [[Client1_29]],
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
          InstanceId = [[Client1_27]],
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
                x = 24370,
                y = -1363,
                z = 0
              }
            },
            {
              InstanceId = [[Client1_801]],
              Class = [[NpcCreature]],
              Angle = 0.5,
              Base = [[palette.entities.creatures.ckdff1]],
              InheritPos = 1,
              Name = [[Gruesome Kincher]],
              ActivitiesId = {
              },
              Behavior = {
                InstanceId = [[Client1_13]],
                Class = [[Behavior]],
                Type = [[]],
                ZoneId = [[]],
                Actions = {
                  {
                    InstanceId = [[Client1_11]],
                    Class = [[LogicEntityAction]],
                    Name = [[]],
                    Actions = {
                      {
                        InstanceId = [[Client1_7]],
                        Class = [[ActionStep]],
                        Entity = r2.RefId([[Client1_802]]),
                        Action = {
                          InstanceId = [[Client1_8]],
                          Class = [[ActionType]],
                          Type = [[Kill]],
                          Value = r2.RefId([[]])
                        }
                      }
                    },
                    Conditions = {
                      {
                        InstanceId = [[Client1_9]],
                        Class = [[ConditionStep]],
                        Entity = r2.RefId([[Client1_802]]),
                        Condition = {
                          InstanceId = [[Client1_10]],
                          Class = [[ConditionType]],
                          Type = [[is alive]],
                          Value = r2.RefId([[]])
                        }
                      }
                    },
                    Event = {
                      InstanceId = [[Client1_12]],
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
                InstanceId = [[Client1_14]],
                Class = [[Position]],
                x = 24380,
                y = -1370,
                z = 0
              }
            },
            {
              InstanceId = [[Client1_802]],
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
                },
                Activities = {
                },
                ChatSequences = {
                }
              },
              Position = {
                InstanceId = [[Client1_16]],
                Class = [[Position]],
                x = 24384,
                y = -1372,
                z = 0
              }
            }
          }
        },
        {
          InstanceId = [[Client1_810]],
          Class = [[ChatSequence]],
          Active = 1,
          AutoStart = 1,
          Base = [[palette.entities.botobjects.dialog]],
          InheritPos = 1,
          Name = [[Dialog 1]],
          Repeating = 0,
          Type = [[None]],
          Behavior = {
            InstanceId = [[Client1_17]],
            Class = [[LogicEntityBehavior]],
            Actions = {
            }
          },
          Components = {
            {
              InstanceId = [[Client1_1]],
              Class = [[ChatStep]],
              Name = [[]],
              Time = 0,
              Actions = {
                {
                  InstanceId = [[Client1_2]],
                  Class = [[ChatAction]],
                  Emote = [[]],
                  Facing = r2.RefId([[]]),
                  Says = [[Client1_850]],
                  Who = r2.RefId([[]]),
                  WhoNoEntity = [[_System]]
                }
              }
            },
            {
              InstanceId = [[Client1_3]],
              Class = [[ChatStep]],
              Name = [[]],
              Time = 3,
              Actions = {
                {
                  InstanceId = [[Client1_4]],
                  Class = [[ChatAction]],
                  Emote = [[]],
                  Facing = r2.RefId([[]]),
                  Says = [[Client1_851]],
                  Who = r2.RefId([[]]),
                  WhoNoEntity = [[_System]]
                }
              }
            }
          },
          Position = {
            InstanceId = [[Client1_18]],
            Class = [[Position]],
            x = 24375,
            y = -1365,
            z = 0
          },
          SubComponents = {
          }
        },
        {
          InstanceId = [[Client1_820]],
          Class = [[Counter]],
          Base = [[palette.entities.botobjects.milestone]],
          InheritPos = 1,
          Name = [[Counter 1]],
          TriggerValue = 0,
          Value = 3,
          _Seed = 1154080571,
          Behavior = {
            InstanceId = [[Client1_19]],
            Class = [[LogicEntityBehavior]],
            Actions = {
            }
          },
          Components = {
          },
          Position = {
            InstanceId = [[Client1_20]],
            Class = [[Position]],
            x = 24376,
            y = -1366,
            z = 0
          }
        },
        {
          InstanceId = [[Client1_830]],
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
            InstanceId = [[Client1_21]],
            Class = [[LogicEntityBehavior]],
            Actions = {
            }
          },
          Components = {
          },
          Position = {
            InstanceId = [[Client1_22]],
            Class = [[Position]],
            x = 24377,
            y = -1367,
            z = 0
          }
        }
      },
      Position = {
        InstanceId = [[Client1_30]],
        Class = [[Position]],
        x = 0,
        y = 0,
        z = 0
      }
    }
  },
  Behavior = {
    InstanceId = [[Client1_32]],
    Class = [[LogicEntityBehavior]],
    Actions = {
    }
  },
  Description = {
    InstanceId = [[Client1_33]],
    Class = [[MapDescription]],
    LevelId = 0,
    OptimalNumberOfPlayer = 0,
    ShortDescription = [[]],
    Title = [[Healthy scenario]]
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
    InstanceId = [[Client1_34]],
    Class = [[Position]],
    x = 0,
    y = 0,
    z = 0
  },
  Texts = {
    InstanceId = [[Client1_35]],
    Class = [[TextManager]],
    Texts = {
      {
        InstanceId = [[Client1_850]],
        Class = [[TextManagerEntry]],
        Count = 1,
        Text = [[The fire is lit.]]
      },
      {
        InstanceId = [[Client1_851]],
        Class = [[TextManagerEntry]],
        Count = 1,
        Text = [[It burns well.]]
      }
    }
  },
  UserComponents = {
  },
  Versions = {
    Act = 6,
    ActionStep = 2,
    ActionType = 0,
    Behavior = 1,
    ChatAction = 0,
    ChatSequence = 1,
    ChatStep = 0,
    ConditionStep = 0,
    ConditionType = 0,
    Counter = 1,
    DefaultFeature = 0,
    EasterEgg = 1,
    EventType = 0,
    Location = 1,
    LogicEntityAction = 0,
    LogicEntityBehavior = 1,
    MapDescription = 0,
    Npc = 1,
    NpcCreature = 1,
    Position = 0,
    Scenario = 4,
    TextManager = 0,
    TextManagerEntry = 0
  }
}
