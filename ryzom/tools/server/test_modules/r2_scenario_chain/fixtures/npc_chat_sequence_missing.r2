scenario = {
  InstanceId = [[Client1_24]],
  Class = [[Scenario]],
  AccessRules = [[liberal]],
  InheritPos = 1,
  Name = [[Npc chat sequence missing]],
  VersionName = [[fixture]],
  Acts = {
    {
      InstanceId = [[Client1_17]],
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
        InstanceId = [[Client1_18]],
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
          InstanceId = [[Client1_16]],
          Class = [[DefaultFeature]],
          Components = {
          }
        }
      },
      Position = {
        InstanceId = [[Client1_19]],
        Class = [[Position]],
        x = 0,
        y = 0,
        z = 0
      }
    },
    {
      InstanceId = [[Client1_21]],
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
        InstanceId = [[Client1_22]],
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
          InstanceId = [[Client1_20]],
          Class = [[DefaultFeature]],
          Components = {
            {
              InstanceId = [[Client1_700]],
              Class = [[NpcCustom]],
              Angle = 0.5,
              Base = [[palette.entities.npcs.civils.m_civil_220]],
              InheritPos = 1,
              Name = [[Zora]],
              ArmColor = 1,
              ArmModel = 6717230,
              EyesColor = 5,
              FeetColor = 1,
              FeetModel = 6715694,
              GabaritArmsWidth = 9,
              GabaritBreastSize = 6,
              GabaritHeight = 1,
              GabaritLegsWidth = 8,
              GabaritTorsoWidth = 4,
              HairColor = 0,
              HairType = 4910,
              HandsColor = 1,
              HandsModel = 6713646,
              JacketColor = 1,
              JacketModel = 6717742,
              Level = 0,
              LinkColor = 0,
              MorphTarget1 = 0,
              MorphTarget2 = 4,
              MorphTarget3 = 3,
              MorphTarget4 = 4,
              MorphTarget5 = 4,
              MorphTarget6 = 0,
              MorphTarget7 = 7,
              MorphTarget8 = 7,
              Notes = [[]],
              Sex = 0,
              Sheet = [[ring_civil_light_melee_blunt_f2.creature]],
              SheetClient = [[basic_matis_male.creature]],
              Speed = 0,
              Tattoo = 19,
              TrouserColor = 1,
              TrouserModel = 6716718,
              WeaponLeftHand = 0,
              WeaponRightHand = 0,
              ActivitiesId = {
              },
              Behavior = {
                InstanceId = [[Client1_14]],
                Class = [[Behavior]],
                Type = [[]],
                ZoneId = [[]],
                Actions = {
                  {
                    InstanceId = [[Client1_6]],
                    Class = [[LogicEntityAction]],
                    Name = [[]],
                    Actions = {
                      {
                        InstanceId = [[Client1_2]],
                        Class = [[ActionStep]],
                        Entity = r2.RefId([[Client1_700]]),
                        Action = {
                          InstanceId = [[Client1_3]],
                          Class = [[ActionType]],
                          Type = [[Sit Down]],
                          Value = r2.RefId([[]])
                        }
                      },
                      {
                        InstanceId = [[Client1_4]],
                        Class = [[ActionStep]],
                        Entity = r2.RefId([[Client1_700]]),
                        Action = {
                          InstanceId = [[Client1_5]],
                          Class = [[ActionType]],
                          Type = [[begin chat sequence]],
                          Value = r2.RefId([[Client1_798]])
                        }
                      }
                    },
                    Conditions = {
                    },
                    Event = {
                      InstanceId = [[Client1_7]],
                      Class = [[EventType]],
                      Type = [[death]],
                      Value = r2.RefId([[]])
                    }
                  },
                  {
                    InstanceId = [[Client1_12]],
                    Class = [[LogicEntityAction]],
                    Name = [[]],
                    Actions = {
                      {
                        InstanceId = [[Client1_8]],
                        Class = [[ActionStep]],
                        Entity = r2.RefId([[Client1_700]]),
                        Action = {
                          InstanceId = [[Client1_9]],
                          Class = [[ActionType]],
                          Type = [[Stand Up]],
                          Value = r2.RefId([[]])
                        }
                      }
                    },
                    Conditions = {
                      {
                        InstanceId = [[Client1_10]],
                        Class = [[ConditionStep]],
                        Entity = r2.RefId([[Client1_700]]),
                        Condition = {
                          InstanceId = [[Client1_11]],
                          Class = [[ConditionType]],
                          Type = [[is in chat sequence]],
                          Value = r2.RefId([[Client1_797]])
                        }
                      }
                    },
                    Event = {
                      InstanceId = [[Client1_13]],
                      Class = [[EventType]],
                      Type = [[death]],
                      Value = r2.RefId([[]])
                    }
                  }
                },
                Activities = {
                  {
                    InstanceId = [[Client1_1]],
                    Class = [[ActivitySequence]],
                    Name = [[]],
                    Repeating = 0,
                    Components = {
                      {
                        InstanceId = [[Client1_740]],
                        Class = [[ActivityStep]],
                        Activity = [[Stand Still]],
                        ActivityZoneId = r2.RefId([[]]),
                        Chat = r2.RefId([[Client1_799]]),
                        Name = [[]],
                        TimeLimit = [[Chat]],
                        TimeLimitValue = [[20]],
                        Type = [[Non Repeating]],
                        EventsIds = {
                        }
                      }
                    }
                  }
                },
                ChatSequences = {
                }
              },
              Position = {
                InstanceId = [[Client1_15]],
                Class = [[Position]],
                x = 24370,
                y = -1363,
                z = 0
              }
            }
          }
        }
      },
      Position = {
        InstanceId = [[Client1_23]],
        Class = [[Position]],
        x = 0,
        y = 0,
        z = 0
      }
    }
  },
  Behavior = {
    InstanceId = [[Client1_25]],
    Class = [[LogicEntityBehavior]],
    Actions = {
    }
  },
  Description = {
    InstanceId = [[Client1_26]],
    Class = [[MapDescription]],
    LevelId = 0,
    OptimalNumberOfPlayer = 0,
    ShortDescription = [[]],
    Title = [[Npc chat sequence missing]]
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
    InstanceId = [[Client1_27]],
    Class = [[Position]],
    x = 0,
    y = 0,
    z = 0
  },
  Texts = {
    InstanceId = [[Client1_28]],
    Class = [[TextManager]],
    Texts = {
    }
  },
  UserComponents = {
  },
  Versions = {
    Act = 3,
    ActionStep = 1,
    ActionType = 0,
    ActivitySequence = 0,
    ActivityStep = 1,
    Behavior = 1,
    ConditionStep = 0,
    ConditionType = 0,
    DefaultFeature = 0,
    EventType = 0,
    Location = 1,
    LogicEntityAction = 0,
    LogicEntityBehavior = 1,
    MapDescription = 0,
    NpcCustom = 0,
    Position = 0,
    Scenario = 2,
    TextManager = 0
  }
}
