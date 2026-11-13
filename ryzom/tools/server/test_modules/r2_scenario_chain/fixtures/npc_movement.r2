scenario = {
  InstanceId = [[Client1_40]],
  Class = [[Scenario]],
  AccessRules = [[liberal]],
  InheritPos = 1,
  Name = [[Npc movement]],
  VersionName = [[fixture]],
  Acts = {
    {
      InstanceId = [[Client1_33]],
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
        InstanceId = [[Client1_34]],
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
          InstanceId = [[Client1_32]],
          Class = [[DefaultFeature]],
          Components = {
          }
        }
      },
      Position = {
        InstanceId = [[Client1_35]],
        Class = [[Position]],
        x = 0,
        y = 0,
        z = 0
      }
    },
    {
      InstanceId = [[Client1_37]],
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
        InstanceId = [[Client1_38]],
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
          InstanceId = [[Client1_36]],
          Class = [[DefaultFeature]],
          Components = {
            {
              InstanceId = [[Client1_860]],
              Class = [[Region]],
              InheritPos = 1,
              Name = [[Place 1]],
              Points = {
                {
                  InstanceId = [[Client1_8]],
                  Class = [[RegionVertex]],
                  InheritPos = 1,
                  Position = {
                    InstanceId = [[Client1_9]],
                    Class = [[Position]],
                    x = 24360,
                    y = -1380,
                    z = 0
                  }
                },
                {
                  InstanceId = [[Client1_10]],
                  Class = [[RegionVertex]],
                  InheritPos = 1,
                  Position = {
                    InstanceId = [[Client1_11]],
                    Class = [[Position]],
                    x = 24370,
                    y = -1380,
                    z = 0
                  }
                },
                {
                  InstanceId = [[Client1_12]],
                  Class = [[RegionVertex]],
                  InheritPos = 1,
                  Position = {
                    InstanceId = [[Client1_13]],
                    Class = [[Position]],
                    x = 24370,
                    y = -1370,
                    z = 0
                  }
                },
                {
                  InstanceId = [[Client1_14]],
                  Class = [[RegionVertex]],
                  InheritPos = 1,
                  Position = {
                    InstanceId = [[Client1_15]],
                    Class = [[Position]],
                    x = 24360,
                    y = -1370,
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
            },
            {
              InstanceId = [[Client1_861]],
              Class = [[Road]],
              InheritPos = 1,
              Name = [[Route 1]],
              Points = {
                {
                  InstanceId = [[Client1_1]],
                  Class = [[WayPoint]],
                  InheritPos = 1,
                  Position = {
                    InstanceId = [[Client1_2]],
                    Class = [[Position]],
                    x = 24390,
                    y = -1380,
                    z = 0
                  }
                },
                {
                  InstanceId = [[Client1_3]],
                  Class = [[WayPoint]],
                  InheritPos = 1,
                  Position = {
                    InstanceId = [[Client1_4]],
                    Class = [[Position]],
                    x = 24400,
                    y = -1380,
                    z = 0
                  }
                },
                {
                  InstanceId = [[Client1_5]],
                  Class = [[WayPoint]],
                  InheritPos = 1,
                  Position = {
                    InstanceId = [[Client1_6]],
                    Class = [[Position]],
                    x = 24410,
                    y = -1375,
                    z = 0
                  }
                }
              },
              Position = {
                InstanceId = [[Client1_7]],
                Class = [[Position]],
                x = 0,
                y = 0,
                z = 0
              }
            },
            {
              InstanceId = [[Client1_17]],
              Class = [[NpcCreature]],
              Angle = 0.5,
              Base = [[palette.entities.creatures.ckdff1]],
              InheritPos = 1,
              Name = [[Gruesome Kincher]],
              ActivitiesId = {
              },
              Behavior = {
                InstanceId = [[Client1_18]],
                Class = [[Behavior]],
                Type = [[wander]],
                ZoneId = [[Client1_860]],
                Actions = {
                },
                Activities = {
                },
                ChatSequences = {
                }
              },
              Position = {
                InstanceId = [[Client1_19]],
                Class = [[Position]],
                x = 24365,
                y = -1375,
                z = 0
              }
            },
            {
              InstanceId = [[Client1_20]],
              Class = [[NpcCreature]],
              Angle = 0.5,
              Base = [[palette.entities.creatures.ckdff1]],
              InheritPos = 1,
              Name = [[Gruesome Kincher]],
              ActivitiesId = {
              },
              Behavior = {
                InstanceId = [[Client1_21]],
                Class = [[Behavior]],
                Type = [[follow_route]],
                ZoneId = [[Client1_861]],
                Actions = {
                },
                Activities = {
                },
                ChatSequences = {
                }
              },
              Position = {
                InstanceId = [[Client1_22]],
                Class = [[Position]],
                x = 24390,
                y = -1380,
                z = 0
              }
            },
            {
              InstanceId = [[Client1_23]],
              Class = [[NpcCreature]],
              Angle = 0.5,
              Base = [[palette.entities.creatures.ckdff1]],
              InheritPos = 1,
              Name = [[Gruesome Kincher]],
              ActivitiesId = {
              },
              Behavior = {
                InstanceId = [[Client1_24]],
                Class = [[Behavior]],
                Type = [[wander]],
                ZoneId = [[Client1_899]],
                Actions = {
                },
                Activities = {
                },
                ChatSequences = {
                }
              },
              Position = {
                InstanceId = [[Client1_25]],
                Class = [[Position]],
                x = 24400,
                y = -1370,
                z = 0
              }
            },
            {
              InstanceId = [[Client1_26]],
              Class = [[NpcCreature]],
              Angle = 0.5,
              Base = [[palette.entities.creatures.ckdff1]],
              InheritPos = 1,
              Name = [[Gruesome Kincher]],
              ActivitiesId = {
              },
              Behavior = {
                InstanceId = [[Client1_27]],
                Class = [[Behavior]],
                Type = [[patrol_route]],
                ZoneId = [[Client1_861]],
                Actions = {
                },
                Activities = {
                },
                ChatSequences = {
                }
              },
              Position = {
                InstanceId = [[Client1_28]],
                Class = [[Position]],
                x = 24391,
                y = -1381,
                z = 0
              }
            },
            {
              InstanceId = [[Client1_29]],
              Class = [[NpcCreature]],
              Angle = 0.5,
              Base = [[palette.entities.creatures.ckdff1]],
              InheritPos = 1,
              Name = [[Gruesome Kincher]],
              ActivitiesId = {
              },
              Behavior = {
                InstanceId = [[Client1_30]],
                Class = [[Behavior]],
                Type = [[repeat_road]],
                ZoneId = [[Client1_861]],
                Actions = {
                },
                Activities = {
                },
                ChatSequences = {
                }
              },
              Position = {
                InstanceId = [[Client1_31]],
                Class = [[Position]],
                x = 24392,
                y = -1382,
                z = 0
              }
            }
          }
        }
      },
      Position = {
        InstanceId = [[Client1_39]],
        Class = [[Position]],
        x = 0,
        y = 0,
        z = 0
      }
    }
  },
  Behavior = {
    InstanceId = [[Client1_41]],
    Class = [[LogicEntityBehavior]],
    Actions = {
    }
  },
  Description = {
    InstanceId = [[Client1_42]],
    Class = [[MapDescription]],
    LevelId = 0,
    OptimalNumberOfPlayer = 0,
    ShortDescription = [[]],
    Title = [[Npc movement]]
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
    InstanceId = [[Client1_43]],
    Class = [[Position]],
    x = 0,
    y = 0,
    z = 0
  },
  Texts = {
    InstanceId = [[Client1_44]],
    Class = [[TextManager]],
    Texts = {
    }
  },
  UserComponents = {
  },
  Versions = {
    Act = 6,
    Behavior = 1,
    DefaultFeature = 0,
    Location = 1,
    LogicEntityBehavior = 1,
    MapDescription = 0,
    NpcCreature = 1,
    Position = 0,
    Region = 1,
    RegionVertex = 1,
    Road = 1,
    Scenario = 4,
    TextManager = 0,
    WayPoint = 1
  }
}
