scenario = {
  InstanceId = [[Client1_34]],
  Class = [[Scenario]],
  AccessRules = [[liberal]],
  InheritPos = 1,
  Name = [[Npc chat time limit]],
  VersionName = [[fixture]],
  Acts = {
    {
      InstanceId = [[Client1_27]],
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
        InstanceId = [[Client1_28]],
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
          InstanceId = [[Client1_26]],
          Class = [[DefaultFeature]],
          Components = {
          }
        }
      },
      Position = {
        InstanceId = [[Client1_29]],
        Class = [[Position]],
        x = 0,
        y = 0,
        z = 0
      }
    },
    {
      InstanceId = [[Client1_31]],
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
        InstanceId = [[Client1_32]],
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
          InstanceId = [[Client1_30]],
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
                InstanceId = [[Client1_22]],
                Class = [[Behavior]],
                Type = [[]],
                ZoneId = [[]],
                Actions = {
                  {
                    InstanceId = [[Client1_1004]],
                    Class = [[LogicEntityAction]],
                    Name = [[]],
                    Actions = {
                      {
                        InstanceId = [[Client1_1002]],
                        Class = [[ActionStep]],
                        Entity = r2.RefId([[Client1_701]]),
                        Action = {
                          InstanceId = [[Client1_1001]],
                          Class = [[ActionType]],
                          Type = [[Sit Down]],
                          Value = r2.RefId([[]])
                        }
                      }
                    },
                    Conditions = {
                    },
                    Event = {
                      InstanceId = [[Client1_1003]],
                      Class = [[EventType]],
                      Type = [[end of chat sequence]],
                      Value = r2.RefId([[Client1_712]])
                    }
                  }
                },
                Activities = {
                  {
                    InstanceId = [[Client1_5]],
                    Class = [[ActivitySequence]],
                    Name = [[]],
                    Repeating = 0,
                    Components = {
                      {
                        InstanceId = [[Client1_740]],
                        Class = [[ActivityStep]],
                        Activity = [[Stand Still]],
                        ActivityZoneId = r2.RefId([[]]),
                        Chat = r2.RefId([[Client1_710]]),
                        Name = [[]],
                        TimeLimit = [[Chat]],
                        TimeLimitValue = [[]],
                        Type = [[Non Repeating]],
                        EventsIds = {
                        }
                      },
                      {
                        InstanceId = [[Client1_741]],
                        Class = [[ActivityStep]],
                        Activity = [[Stand Still]],
                        ActivityZoneId = r2.RefId([[]]),
                        Chat = r2.RefId([[Client1_711]]),
                        Name = [[]],
                        TimeLimit = [[Chat]],
                        TimeLimitValue = [[]],
                        Type = [[Non Repeating]],
                        EventsIds = {
                        }
                      },
                      {
                        InstanceId = [[Client1_742]],
                        Class = [[ActivityStep]],
                        Activity = [[Stand Still]],
                        ActivityZoneId = r2.RefId([[]]),
                        Chat = r2.RefId([[Client1_712]]),
                        Name = [[]],
                        TimeLimit = [[Chat]],
                        TimeLimitValue = [[]],
                        Type = [[Non Repeating]],
                        EventsIds = {
                        }
                      },
                      {
                        InstanceId = [[Client1_743]],
                        Class = [[ActivityStep]],
                        Activity = [[Stand Still]],
                        ActivityZoneId = r2.RefId([[]]),
                        Chat = r2.RefId([[Client1_713]]),
                        Name = [[]],
                        TimeLimit = [[Chat]],
                        TimeLimitValue = [[]],
                        Type = [[Non Repeating]],
                        EventsIds = {
                        }
                      }
                    }
                  }
                },
                ChatSequences = {
                  {
                    InstanceId = [[Client1_710]],
                    Class = [[ChatSequence]],
                    Name = [[Line]],
                    Repeating = 0,
                    Components = {
                      {
                        InstanceId = [[Client1_720]],
                        Class = [[ChatStep]],
                        Name = [[]],
                        Time = 5,
                        Actions = {
                          {
                            InstanceId = [[Client1_1005]],
                            Class = [[ChatAction]],
                            Emote = [[]],
                            Facing = r2.RefId([[]]),
                            Says = [[Client1_730]],
                            Who = r2.RefId([[Client1_700]])
                          }
                        }
                      }
                    }
                  },
                  {
                    InstanceId = [[Client1_711]],
                    Class = [[ChatSequence]],
                    Name = [[Turn]],
                    Repeating = 0,
                    Components = {
                      {
                        InstanceId = [[Client1_721]],
                        Class = [[ChatStep]],
                        Name = [[]],
                        Time = 2,
                        Actions = {
                          {
                            InstanceId = [[Client1_1006]],
                            Class = [[ChatAction]],
                            Emote = [[]],
                            Facing = r2.RefId([[]]),
                            Says = [[Client1_731]],
                            Who = r2.RefId([[Client1_700]])
                          }
                        }
                      },
                      {
                        InstanceId = [[Client1_722]],
                        Class = [[ChatStep]],
                        Name = [[]],
                        Time = 3,
                        Actions = {
                          {
                            InstanceId = [[Client1_1007]],
                            Class = [[ChatAction]],
                            Emote = [[]],
                            Facing = r2.RefId([[Client1_700]]),
                            Says = [[Client1_732]],
                            Who = r2.RefId([[Client1_701]])
                          }
                        }
                      }
                    }
                  },
                  {
                    InstanceId = [[Client1_712]],
                    Class = [[ChatSequence]],
                    Name = [[Ended]],
                    Repeating = 0,
                    Components = {
                      {
                        InstanceId = [[Client1_723]],
                        Class = [[ChatStep]],
                        Name = [[]],
                        Time = 5,
                        Actions = {
                          {
                            InstanceId = [[Client1_1008]],
                            Class = [[ChatAction]],
                            Emote = [[]],
                            Facing = r2.RefId([[]]),
                            Says = [[Client1_733]],
                            Who = r2.RefId([[Client1_700]])
                          }
                        }
                      }
                    }
                  },
                  {
                    InstanceId = [[Client1_713]],
                    Class = [[ChatSequence]],
                    Name = [[Long]],
                    Repeating = 0,
                    Components = {
                      {
                        InstanceId = [[Client1_724]],
                        Class = [[ChatStep]],
                        Name = [[]],
                        Time = 40000,
                        Actions = {
                          {
                            InstanceId = [[Client1_1009]],
                            Class = [[ChatAction]],
                            Emote = [[]],
                            Facing = r2.RefId([[]]),
                            Says = [[Client1_734]],
                            Who = r2.RefId([[Client1_700]])
                          }
                        }
                      }
                    }
                  }
                }
              },
              Position = {
                InstanceId = [[Client1_23]],
                Class = [[Position]],
                x = 24370,
                y = -1363,
                z = 0
              }
            },
            {
              InstanceId = [[Client1_701]],
              Class = [[NpcCustom]],
              Angle = 0.5,
              Base = [[palette.entities.npcs.civils.m_civil_220]],
              InheritPos = 1,
              Name = [[Tolo]],
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
                InstanceId = [[Client1_24]],
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
                InstanceId = [[Client1_25]],
                Class = [[Position]],
                x = 24372,
                y = -1363,
                z = 0
              }
            }
          }
        }
      },
      Position = {
        InstanceId = [[Client1_33]],
        Class = [[Position]],
        x = 0,
        y = 0,
        z = 0
      }
    }
  },
  Behavior = {
    InstanceId = [[Client1_35]],
    Class = [[LogicEntityBehavior]],
    Actions = {
    }
  },
  Description = {
    InstanceId = [[Client1_36]],
    Class = [[MapDescription]],
    LevelId = 0,
    OptimalNumberOfPlayer = 0,
    ShortDescription = [[]],
    Title = [[Npc chat time limit]]
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
    InstanceId = [[Client1_37]],
    Class = [[Position]],
    x = 0,
    y = 0,
    z = 0
  },
  Texts = {
    InstanceId = [[Client1_38]],
    Class = [[TextManager]],
    Texts = {
      {
        InstanceId = [[Client1_730]],
        Class = [[TextManagerEntry]],
        Count = 1,
        Text = [[One line, after a while.]]
      },
      {
        InstanceId = [[Client1_731]],
        Class = [[TextManagerEntry]],
        Count = 1,
        Text = [[Tolo?]]
      },
      {
        InstanceId = [[Client1_732]],
        Class = [[TextManagerEntry]],
        Count = 1,
        Text = [[Yes, Zora?]]
      },
      {
        InstanceId = [[Client1_733]],
        Class = [[TextManagerEntry]],
        Count = 1,
        Text = [[Sit down when I am done.]]
      },
      {
        InstanceId = [[Client1_734]],
        Class = [[TextManagerEntry]],
        Count = 1,
        Text = [[This one takes half a day to come.]]
      }
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
    ChatAction = 0,
    ChatSequence = 0,
    ChatStep = 0,
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
    TextManager = 0,
    TextManagerEntry = 0
  }
}
