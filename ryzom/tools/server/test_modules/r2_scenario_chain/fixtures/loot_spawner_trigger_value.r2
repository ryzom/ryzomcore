scenario = {
  InstanceId = [[Client1_17]],
  Class = [[Scenario]],
  AccessRules = [[liberal]],
  InheritPos = 1,
  Name = [[Loot spawner trigger value]],
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
              InstanceId = [[Client1_830]],
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
                x = 24380,
                y = -1370,
                z = 0
              }
            },
            {
              InstanceId = [[Client1_831]],
              Class = [[NpcCreature]],
              Angle = 0.5,
              Base = [[palette.entities.creatures.ckdff1]],
              InheritPos = 1,
              Name = [[Gruesome Kincher]],
              ActivitiesId = {
              },
              Behavior = {
                InstanceId = [[Client1_7]],
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
                InstanceId = [[Client1_8]],
                Class = [[Position]],
                x = 24384,
                y = -1372,
                z = 0
              }
            }
          }
        },
        {
          InstanceId = [[Client1_833]],
          Class = [[LootSpawner]],
          Active = 1,
          Base = [[palette.entities.botobjects.user_event]],
          EasterEggId = [[Client1_832]],
          InheritPos = 1,
          Name = [[Loot spawner 1]],
          Npc1Id = r2.RefId([[Client1_830]]),
          Npc2Id = r2.RefId([[Client1_831]]),
          Npc3Id = r2.RefId([[]]),
          Npc4Id = r2.RefId([[]]),
          Npc5Id = r2.RefId([[]]),
          NpcNumber = 2,
          _TriggerValue = 2,
          _Seed = 1154080571,
          Behavior = {
            InstanceId = [[Client1_1]],
            Class = [[LogicEntityBehavior]],
            Actions = {
            }
          },
          Components = {
            {
              InstanceId = [[Client1_832]],
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
                InstanceId = [[Client1_2]],
                Class = [[LogicEntityBehavior]],
                Actions = {
                }
              },
              Components = {
              },
              Position = {
                InstanceId = [[Client1_3]],
                Class = [[Position]],
                x = 1,
                y = 0,
                z = 0
              }
            }
          },
          Ghosts = {
          },
          Position = {
            InstanceId = [[Client1_4]],
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
    Title = [[Loot spawner trigger value]]
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
    Behavior = 1,
    DefaultFeature = 0,
    EasterEgg = 1,
    Location = 1,
    LogicEntityBehavior = 1,
    LootSpawner = 1,
    MapDescription = 0,
    NpcCreature = 1,
    Position = 0,
    Scenario = 4,
    TextManager = 0
  }
}
