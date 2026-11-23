scenario = {
  InstanceId = [[Client1_31]],
  Class = [[Scenario]],
  AccessRules = [[liberal]],
  InheritPos = 1,
  Name = [[Control characters]],
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
      Name = [[Actone]],
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
              InstanceId = [[Client1_801]],
              Class = [[NpcCustom]],
              Angle = 0.5,
              Base = [[palette.entities.npcs.civils.m_civil_220]],
              InheritPos = 1,
              Name = [[Bob]],
              ActivitiesId = {
              },
              Behavior = {
                InstanceId = [[Client1_13]],
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
                InstanceId = [[Client1_14]],
                Class = [[Position]],
                x = 24380,
                y = -1370,
                z = 0
              }
            },
            {
              InstanceId = [[Client1_802]],
              Class = [[NpcCustom]],
              Angle = 0.5,
              Base = [[palette.entities.npcs.civils.m_civil_220]],
              InheritPos = 1,
              Name = [[Ann]],
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
            InstanceId = [[Client1_50]],
            Class = [[LogicEntityBehavior]],
            Actions = {
            }
          },
          Components = {
            {
              InstanceId = [[Client1_51]],
              Class = [[ChatStep]],
              Name = [[]],
              Time = 0,
              Actions = {
                {
                  InstanceId = [[Client1_52]],
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
            InstanceId = [[Client1_53]],
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
          Class = [[TalkTo]],
          Active = 1,
          Base = [[palette.entities.botobjects.bot_chat]],
          InheritPos = 1,
          Name = [[Talk]],
          ContextualText = [[Ask <mission_target>]],
          MissionGiver = r2.RefId([[Client1_801]]),
          MissionSucceedText = [[Welldone]],
          MissionTarget = r2.RefId([[Client1_802]]),
          MissionText = [[Find<mission_target>	now]],
          Repeatable = 0,
          WaitValidationText = [[Hello]],
          _Seed = 1154302056,
          Behavior = {
            InstanceId = [[Client1_40]],
            Class = [[LogicEntityBehavior]],
            Actions = {
            }
          },
          Components = {
          },
          Position = {
            InstanceId = [[Client1_41]],
            Class = [[Position]],
            x = 24376,
            y = -1366,
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
    Title = [[Control characters]]
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
      Name = [[Oldkey]],
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
      {
        InstanceId = [[Client1_850]],
        Class = [[TextManagerEntry]],
        Count = 1,
        Text = [[Ringthe	bellnow]]
      }
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
    TalkTo = 1,
    ChatSequence = 1,
    ChatStep = 0,
    ChatAction = 0
  }
}
