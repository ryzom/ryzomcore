scenario = {
  InstanceId = [[Client1_25]],
  Class = [[Scenario]],
  AccessRules = [[liberal]],
  InheritPos = 1,
  Name = [[Proximity dialog inactive]],
  VersionName = [[fixture]],
  Acts = {
    {
      InstanceId = [[Client1_18]],
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
        InstanceId = [[Client1_19]],
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
          InstanceId = [[Client1_17]],
          Class = [[DefaultFeature]],
          Components = {
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
    },
    {
      InstanceId = [[Client1_22]],
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
            {
              InstanceId = [[Client1_14]],
              Class = [[Npc]],
              Angle = 0.5,
              Base = [[palette.entities.botobjects.campfire]],
              InheritPos = 1,
              Name = [[camp fire 1]],
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
                x = 24370,
                y = -1363,
                z = 0
              }
            }
          }
        },
        {
          InstanceId = [[Client1_862]],
          Class = [[ProximityDialog]],
          Active = 0,
          AutoStart = 0,
          Base = [[palette.entities.botobjects.dialog]],
          InheritPos = 1,
          Name = [[Proximity dialog 1]],
          Repeating = 0,
          TriggerOn = 1,
          Type = [[None]],
          _zoneTriggerId = [[Client1_861]],
          Behavior = {
            InstanceId = [[Client1_12]],
            Class = [[LogicEntityBehavior]],
            Actions = {
            }
          },
          Components = {
          },
          Position = {
            InstanceId = [[Client1_13]],
            Class = [[Position]],
            x = 24375,
            y = -1365,
            z = 0
          },
          SubComponents = {
            {
              InstanceId = [[Client1_861]],
              Class = [[ZoneTrigger]],
              Active = 1,
              Base = [[palette.entities.botobjects.trigger_zone]],
              Cyclic = 1,
              InheritPos = 0,
              Name = [[Proximity dialog 1 zone trigger]],
              _Zone = [[Client1_860]],
              Behavior = {
                InstanceId = [[Client1_10]],
                Class = [[LogicEntityBehavior]],
                Actions = {
                }
              },
              Components = {
                {
                  InstanceId = [[Client1_860]],
                  Class = [[Region]],
                  InheritPos = 1,
                  Name = [[Zone 1]],
                  Points = {
                    {
                      InstanceId = [[Client1_1]],
                      Class = [[RegionVertex]],
                      InheritPos = 1,
                      Position = {
                        InstanceId = [[Client1_2]],
                        Class = [[Position]],
                        x = -5,
                        y = -5,
                        z = 0
                      }
                    },
                    {
                      InstanceId = [[Client1_3]],
                      Class = [[RegionVertex]],
                      InheritPos = 1,
                      Position = {
                        InstanceId = [[Client1_4]],
                        Class = [[Position]],
                        x = 5,
                        y = -5,
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
                        x = 5,
                        y = 5,
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
                        x = -5,
                        y = 5,
                        z = 0
                      }
                    }
                  },
                  Position = {
                    InstanceId = [[Client1_9]],
                    Class = [[Position]],
                    x = 0,
                    y = 0,
                    z = 0
                  }
                }
              },
              Position = {
                InstanceId = [[Client1_11]],
                Class = [[Position]],
                x = 24375,
                y = -1365,
                z = 0
              }
            }
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
    }
  },
  Behavior = {
    InstanceId = [[Client1_26]],
    Class = [[LogicEntityBehavior]],
    Actions = {
    }
  },
  Description = {
    InstanceId = [[Client1_27]],
    Class = [[MapDescription]],
    LevelId = 0,
    OptimalNumberOfPlayer = 0,
    ShortDescription = [[]],
    Title = [[Proximity dialog inactive]]
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
    InstanceId = [[Client1_28]],
    Class = [[Position]],
    x = 0,
    y = 0,
    z = 0
  },
  Texts = {
    InstanceId = [[Client1_29]],
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
    Npc = 1,
    Position = 0,
    ProximityDialog = 1,
    Region = 1,
    RegionVertex = 1,
    Scenario = 4,
    TextManager = 0,
    ZoneTrigger = 1
  }
}
