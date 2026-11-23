scenario = {
  InstanceId = [[Client1_31]],
  Class = [[Scenario]],
  AccessRules = [[liberal]],
  InheritPos = 1,
  Name = [[Percent signs]],
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
              InstanceId = [[Client1_801]],
              Class = [[NpcCustom]],
              Angle = 0.5,
              Base = [[palette.entities.npcs.civils.m_civil_220]],
              InheritPos = 1,
              Name = [[Bob 100%]],
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
              Name = [[Ann %1 %d]],
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
            },
            {
              InstanceId = [[Client1_803]],
              Class = [[Npc]],
              Angle = 0.5,
              Base = [[palette.entities.botobjects.campfire]],
              InheritPos = 1,
              Name = [[Fire 5%]],
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
          InstanceId = [[Client1_810]],
          Class = [[TalkTo]],
          Active = 1,
          Base = [[palette.entities.botobjects.bot_chat]],
          InheritPos = 1,
          Name = [[Talk 50%]],
          ContextualText = [[Ask <mission_target> (100%)]],
          MissionGiver = r2.RefId([[Client1_801]]),
          MissionSucceedText = [[<mission_target> is 100% sure]],
          MissionTarget = r2.RefId([[Client1_802]]),
          MissionText = [[<mission_giver> pays 50% more to who finds <mission_target>]],
          Repeatable = 0,
          WaitValidationText = [[<mission_giver> sent you? 10%, no more]],
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
        },
        {
          InstanceId = [[Client1_820]],
          Class = [[SceneryObjectInteraction]],
          Active = 1,
          Base = [[palette.entities.botobjects.bot_chat]],
          InheritPos = 1,
          Name = [[Look 5%]],
          ContextualText = [[Light <object_name>]],
          Repeatable = 0,
          SceneryObject = r2.RefId([[Client1_803]]),
          _Seed = 1154302057,
          Behavior = {
            InstanceId = [[Client1_42]],
            Class = [[LogicEntityBehavior]],
            Actions = {
            }
          },
          Components = {
          },
          Position = {
            InstanceId = [[Client1_43]],
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
    Title = [[Percent signs]]
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
    SceneryObjectInteraction = 0
  }
}
