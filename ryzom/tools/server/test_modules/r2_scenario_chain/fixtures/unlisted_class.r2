scenario = {
  InstanceId = [[Client1_18]],
  Class = [[Scenario]],
  AccessRules = [[liberal]],
  InheritPos = 1,
  Name = [[Unlisted class]],
  VersionName = [[fixture]],
  Acts = {
    {
      InstanceId = [[Client1_11]],
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
        InstanceId = [[Client1_12]],
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
          InstanceId = [[Client1_10]],
          Class = [[DefaultFeature]],
          Components = {
          }
        }
      },
      Position = {
        InstanceId = [[Client1_13]],
        Class = [[Position]],
        x = 0,
        y = 0,
        z = 0
      }
    },
    {
      InstanceId = [[Client1_15]],
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
        InstanceId = [[Client1_16]],
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
          InstanceId = [[Client1_14]],
          Class = [[DefaultFeature]],
          Components = {
            {
              InstanceId = [[Client1_1]],
              Class = [[Npc]],
              Angle = 0.5,
              Base = [[palette.entities.botobjects.campfire]],
              InheritPos = 1,
              Name = [[camp fire 1]],
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
                x = 24370,
                y = -1363,
                z = 0
              }
            }
          }
        },
        {
          InstanceId = [[Client1_851]],
          Class = [[ChatSequence]],
          Active = 0,
          AutoStart = 1,
          Base = [[palette.entities.botobjects.dialog]],
          InheritPos = 1,
          Name = [[Dialog 1]],
          Repeating = 0,
          Type = [[None]],
          Behavior = {
            InstanceId = [[Client1_6]],
            Class = [[LogicEntityBehavior]],
            Actions = {
            }
          },
          Components = {
            {
              InstanceId = [[Client1_4]],
              Class = [[ChatStep]],
              Name = [[]],
              Time = 3,
              Actions = {
                {
                  InstanceId = [[Client1_5]],
                  Class = [[ChatAction]],
                  Emote = [[]],
                  Facing = r2.RefId([[]]),
                  Says = [[Client1_850]],
                  Who = r2.RefId([[]]),
                  WhoNoEntity = [[_System]]
                }
              }
            }
          },
          Position = {
            InstanceId = [[Client1_7]],
            Class = [[Position]],
            x = 24375,
            y = -1365,
            z = 0
          },
          SubComponents = {
          }
        },
        {
          InstanceId = [[Client1_852]],
          Class = [[Counter]],
          Base = [[palette.entities.botobjects.milestone]],
          InheritPos = 1,
          Name = [[Counter 1]],
          TriggerValue = 0,
          Value = 3,
          _Seed = 1154080571,
          Behavior = {
            InstanceId = [[Client1_8]],
            Class = [[LogicEntityBehavior]],
            Actions = {
            }
          },
          Components = {
          },
          Position = {
            InstanceId = [[Client1_9]],
            Class = [[Position]],
            x = 24376,
            y = -1366,
            z = 0
          }
        }
      },
      Position = {
        InstanceId = [[Client1_17]],
        Class = [[Position]],
        x = 0,
        y = 0,
        z = 0
      }
    }
  },
  Behavior = {
    InstanceId = [[Client1_19]],
    Class = [[LogicEntityBehavior]],
    Actions = {
    }
  },
  Description = {
    InstanceId = [[Client1_20]],
    Class = [[MapDescription]],
    LevelId = 0,
    OptimalNumberOfPlayer = 0,
    ShortDescription = [[]],
    Title = [[Unlisted class]]
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
    InstanceId = [[Client1_21]],
    Class = [[Position]],
    x = 0,
    y = 0,
    z = 0
  },
  Texts = {
    InstanceId = [[Client1_22]],
    Class = [[TextManager]],
    Texts = {
      {
        InstanceId = [[Client1_850]],
        Class = [[TextManagerEntry]],
        Count = 1,
        Text = [[The fire is lit.]]
      }
    }
  },
  UserComponents = {
  },
  Versions = {
    Act = 6,
    Behavior = 1,
    ChatAction = 0,
    ChatStep = 0,
    DefaultFeature = 0,
    Location = 1,
    LogicEntityBehavior = 1,
    MapDescription = 0,
    Npc = 1,
    Position = 0,
    Scenario = 4,
    TextManager = 0,
    TextManagerEntry = 0
  }
}
