scenario = {
  InstanceId = [[Client1_23]],
  Class = [[Scenario]],
  AccessRules = [[liberal]],
  InheritPos = 1,
  Name = [[Unlisted bandit camp]],
  VersionName = [[fixture]],
  Acts = {
    {
      InstanceId = [[Client1_16]],
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
        InstanceId = [[Client1_17]],
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
          InstanceId = [[Client1_15]],
          Class = [[DefaultFeature]],
          Components = {
          }
        }
      },
      Position = {
        InstanceId = [[Client1_18]],
        Class = [[Position]],
        x = 0,
        y = 0,
        z = 0
      }
    },
    {
      InstanceId = [[Client1_20]],
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
            {
              InstanceId = [[Client1_1]],
              Class = [[NpcCreature]],
              Angle = 0.5,
              Base = [[palette.entities.creatures.ckdff1]],
              InheritPos = 1,
              Name = [[Gruesome Kincher]],
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
                x = 24380,
                y = -1370,
                z = 0
              }
            }
          }
        },
        {
          InstanceId = [[Client1_870]],
          Class = [[BanditCamp]],
          BanditsCount = [[3]],
          BanditsLevel = [[170]],
          Base = [[palette.entities.botobjects.campfire]],
          InheritPos = 1,
          Name = [[Bandit camp 1]],
          Race = [[Zorai]],
          ZoneSize = [[10]],
          _BanditsCount = 3,
          _BanditsLevel = 3,
          _Race = 3,
          _Seed = 1154080571,
          _Zone = [[Client1_875]],
          Active = 1,
          Cost = 0,
          NoRespawn = 0,
          SitDuration = 20,
          WanderDuration = 20,
          Behavior = {
            InstanceId = [[Client1_4]],
            Class = [[LogicEntityBehavior]],
            Actions = {
            }
          },
          Components = {
            {
              InstanceId = [[Client1_875]],
              Class = [[Region]],
              InheritPos = 0,
              Name = [[Wander zone 1]],
              Points = {
                {
                  InstanceId = [[Client1_5]],
                  Class = [[RegionVertex]],
                  InheritPos = 1,
                  Position = {
                    InstanceId = [[Client1_6]],
                    Class = [[Position]],
                    x = 24370,
                    y = -1370,
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
                    x = 24380,
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
                    x = 24380,
                    y = -1360,
                    z = 0
                  }
                },
                {
                  InstanceId = [[Client1_11]],
                  Class = [[RegionVertex]],
                  InheritPos = 1,
                  Position = {
                    InstanceId = [[Client1_12]],
                    Class = [[Position]],
                    x = 24370,
                    y = -1360,
                    z = 0
                  }
                }
              },
              Position = {
                InstanceId = [[Client1_13]],
                Class = [[Position]],
                x = 0,
                y = 0,
                z = 0
              },
              Deletable = 0
            }
          },
          Ghosts = {
          },
          Position = {
            InstanceId = [[Client1_14]],
            Class = [[Position]],
            x = 24375,
            y = -1365,
            z = 0
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
    }
  },
  Behavior = {
    InstanceId = [[Client1_24]],
    Class = [[LogicEntityBehavior]],
    Actions = {
    }
  },
  Description = {
    InstanceId = [[Client1_25]],
    Class = [[MapDescription]],
    LevelId = 0,
    OptimalNumberOfPlayer = 0,
    ShortDescription = [[]],
    Title = [[Unlisted bandit camp]]
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
    InstanceId = [[Client1_26]],
    Class = [[Position]],
    x = 0,
    y = 0,
    z = 0
  },
  Texts = {
    InstanceId = [[Client1_27]],
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
    Scenario = 4,
    TextManager = 0
  }
}
