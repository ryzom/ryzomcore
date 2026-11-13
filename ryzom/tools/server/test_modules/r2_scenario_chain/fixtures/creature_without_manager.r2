scenario = {
  InstanceId = [[Client1_29]],
  Class = [[Scenario]],
  AccessRules = [[liberal]],
  InheritPos = 1,
  Name = [[Creature without manager]],
  VersionName = [[fixture]],
  Acts = {
    {
      InstanceId = [[Client1_22]],
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
        InstanceId = [[Client1_23]],
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
          InstanceId = [[Client1_21]],
          Class = [[DefaultFeature]],
          Components = {
          }
        }
      },
      Position = {
        InstanceId = [[Client1_24]],
        Class = [[Position]],
        x = 0,
        y = 0,
        z = 0
      }
    },
    {
      InstanceId = [[Client1_26]],
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
        InstanceId = [[Client1_27]],
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
          InstanceId = [[Client1_25]],
          Class = [[DefaultFeature]],
          Components = {
            {
              InstanceId = [[Client1_840]],
              Class = [[Region]],
              InheritPos = 1,
              Name = [[Sleep zone 1]],
              Points = {
                {
                  InstanceId = [[Client1_3]],
                  Class = [[RegionVertex]],
                  InheritPos = 1,
                  Position = {
                    InstanceId = [[Client1_4]],
                    Class = [[Position]],
                    x = 24360,
                    y = -1380,
                    z = 0
                  }
                },
                {
                  InstanceId = [[Client1_5]],
                  Class = [[RegionVertex]],
                  InheritPos = 1,
                  Position = {
                    InstanceId = [[Client1_6]],
                    Class = [[Position]],
                    x = 24370,
                    y = -1380,
                    z = 0
                  }
                },
                {
                  InstanceId = [[Client1_7]],
                  Class = [[RegionVertex]],
                  InheritPos = 1,
                  Position = {
                    InstanceId = [[Client1_8]],
                    Class = [[Position]],
                    x = 24370,
                    y = -1370,
                    z = 0
                  }
                },
                {
                  InstanceId = [[Client1_9]],
                  Class = [[RegionVertex]],
                  InheritPos = 1,
                  Position = {
                    InstanceId = [[Client1_10]],
                    Class = [[Position]],
                    x = 24360,
                    y = -1370,
                    z = 0
                  }
                }
              },
              Position = {
                InstanceId = [[Client1_11]],
                Class = [[Position]],
                x = 0,
                y = 0,
                z = 0
              }
            },
            {
              InstanceId = [[Client1_841]],
              Class = [[Region]],
              InheritPos = 1,
              Name = [[Food zone 1]],
              Points = {
                {
                  InstanceId = [[Client1_12]],
                  Class = [[RegionVertex]],
                  InheritPos = 1,
                  Position = {
                    InstanceId = [[Client1_13]],
                    Class = [[Position]],
                    x = 24390,
                    y = -1380,
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
                    x = 24400,
                    y = -1380,
                    z = 0
                  }
                },
                {
                  InstanceId = [[Client1_16]],
                  Class = [[RegionVertex]],
                  InheritPos = 1,
                  Position = {
                    InstanceId = [[Client1_17]],
                    Class = [[Position]],
                    x = 24400,
                    y = -1370,
                    z = 0
                  }
                },
                {
                  InstanceId = [[Client1_18]],
                  Class = [[RegionVertex]],
                  InheritPos = 1,
                  Position = {
                    InstanceId = [[Client1_19]],
                    Class = [[Position]],
                    x = 24390,
                    y = -1370,
                    z = 0
                  }
                }
              },
              Position = {
                InstanceId = [[Client1_20]],
                Class = [[Position]],
                x = 0,
                y = 0,
                z = 0
              }
            }
          }
        },
        {
          InstanceId = [[Client1_842]],
          Class = [[Creature]],
          Base = [[palette.entities.botobjects.user_event]],
          CrittersCount = [[3]],
          FoodDuration = 30,
          FoodZone = r2.RefId([[Client1_841]]),
          InheritPos = 1,
          Name = [[Herd 1]],
          RaceBase = [[palette.entities.creatures.ckdff1]],
          RaceName = [[Gruesome Kincher]],
          SleepDuration = 30,
          SleepZone = r2.RefId([[Client1_840]]),
          _CrittersCount = 3,
          _Seed = 1154080571,
          Behavior = {
            InstanceId = [[Client1_1]],
            Class = [[LogicEntityBehavior]],
            Actions = {
            }
          },
          Components = {
          },
          Ghosts = {
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
        InstanceId = [[Client1_28]],
        Class = [[Position]],
        x = 0,
        y = 0,
        z = 0
      }
    }
  },
  Behavior = {
    InstanceId = [[Client1_30]],
    Class = [[LogicEntityBehavior]],
    Actions = {
    }
  },
  Description = {
    InstanceId = [[Client1_31]],
    Class = [[MapDescription]],
    LevelId = 0,
    OptimalNumberOfPlayer = 0,
    ShortDescription = [[]],
    Title = [[Creature without manager]]
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
    InstanceId = [[Client1_32]],
    Class = [[Position]],
    x = 0,
    y = 0,
    z = 0
  },
  Texts = {
    InstanceId = [[Client1_33]],
    Class = [[TextManager]],
    Texts = {
    }
  },
  UserComponents = {
  },
  Versions = {
    Act = 6,
    Creature = 1,
    DefaultFeature = 0,
    Location = 1,
    LogicEntityBehavior = 1,
    MapDescription = 0,
    Position = 0,
    Region = 1,
    RegionVertex = 1,
    Scenario = 4,
    TextManager = 0
  }
}
