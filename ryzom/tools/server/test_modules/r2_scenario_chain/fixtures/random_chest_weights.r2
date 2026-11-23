scenario = {
  InstanceId = [[Client1_31]],
  Class = [[Scenario]],
  AccessRules = [[liberal]],
  InheritPos = 1,
  Name = [[Random chest weights]],
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
              InstanceId = [[Client1_803]],
              Class = [[Npc]],
              Angle = 0.5,
              Base = [[palette.entities.botobjects.campfire]],
              InheritPos = 1,
              Name = [[camp fire 1]],
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
                x = 24388,
                y = -1374,
                z = 0
              }
            }
          }
        },
        {
          InstanceId = [[Client1_830]],
          Class = [[RandomChest]],
          Active = 1,
          Base = [[palette.entities.botobjects.chest_wisdom_std_sel]],
          InheritPos = 1,
          Name = [[Chest 1]],
          Item1Id = r2.RefId([[Client1_860]]),
          Item1Qty = 1,
          Item1Weight = 25,
          Item2Id = r2.RefId([[Client1_861]]),
          Item2Qty = 2,
          Item2Weight = 25,
          Item3Id = r2.RefId([[Client1_862]]),
          Item3Qty = 3,
          Item3Weight = 50,
          ItemNumber = 3,
          _Seed = 1154302058,
          Behavior = {
            InstanceId = [[Client1_44]],
            Class = [[LogicEntityBehavior]],
            Actions = {
            }
          },
          Components = {
          },
          Position = {
            InstanceId = [[Client1_45]],
            Class = [[Position]],
            x = 24377,
            y = -1367,
            z = 0
          }
        },
        {
          InstanceId = [[Client1_831]],
          Class = [[RandomChest]],
          Active = 1,
          Base = [[palette.entities.botobjects.chest_wisdom_std_sel]],
          InheritPos = 1,
          Name = [[Chest 2]],
          Item1Id = r2.RefId([[Client1_860]]),
          Item1Qty = 1,
          Item1Weight = 33,
          Item2Id = r2.RefId([[Client1_861]]),
          Item2Qty = 1,
          Item2Weight = 33,
          Item3Id = r2.RefId([[Client1_862]]),
          Item3Qty = 1,
          Item3Weight = 33,
          ItemNumber = 3,
          _Seed = 1154302059,
          Behavior = {
            InstanceId = [[Client1_46]],
            Class = [[LogicEntityBehavior]],
            Actions = {
            }
          },
          Components = {
          },
          Position = {
            InstanceId = [[Client1_47]],
            Class = [[Position]],
            x = 24378,
            y = -1368,
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
    Title = [[Random chest weights]]
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
    {
      InstanceId = [[Client1_860]],
      Class = [[PlotItem]],
      Comment = [[]],
      Desc = [[]],
      Name = [[Key]],
      SheetId = 8632366
    },
    {
      InstanceId = [[Client1_861]],
      Class = [[PlotItem]],
      Comment = [[]],
      Desc = [[]],
      Name = [[Map]],
      SheetId = 8632366
    },
    {
      InstanceId = [[Client1_862]],
      Class = [[PlotItem]],
      Comment = [[]],
      Desc = [[]],
      Name = [[Coin]],
      SheetId = 8632366
    }
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
    NpcCustom = 0,
    Npc = 1,
    PlotItem = 0,
    Position = 0,
    Scenario = 4,
    TextManager = 0,
    TextManagerEntry = 0,
    RandomChest = 0
  }
}
