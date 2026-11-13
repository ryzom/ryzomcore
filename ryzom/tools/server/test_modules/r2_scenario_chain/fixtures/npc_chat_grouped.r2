scenario = {
  InstanceId = [[Client1_17]],
  Class = [[Scenario]],
  AccessRules = [[liberal]],
  InheritPos = 1,
  Name = [[Npc chat grouped]],
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
              InstanceId = [[Client1_750]],
              Class = [[NpcGrpFeature]],
              InheritPos = 1,
              Name = [[Group 1]],
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
                  {
                    InstanceId = [[Client1_3]],
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
                        TimeLimit = [[Few Sec]],
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
                    InstanceId = [[Client1_4]],
                    Class = [[Behavior]],
                    Type = [[]],
                    ZoneId = [[]],
                    Actions = {
                    },
                    Activities = {
                    },
                    ChatSequences = {
                      {
                        InstanceId = [[Client1_710]],
                        Class = [[ChatSequence]],
                        Name = [[Orders]],
                        Repeating = 1,
                        Components = {
                          {
                            InstanceId = [[Client1_720]],
                            Class = [[ChatStep]],
                            Name = [[]],
                            Time = 2,
                            Actions = {
                              {
                                InstanceId = [[Client1_1]],
                                Class = [[ChatAction]],
                                Emote = [[]],
                                Facing = r2.RefId([[]]),
                                Says = [[Client1_730]],
                                Who = r2.RefId([[Client1_700]])
                              }
                            }
                          }
                        }
                      }
                    }
                  },
                  Position = {
                    InstanceId = [[Client1_5]],
                    Class = [[Position]],
                    x = 0,
                    y = 0,
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
                    InstanceId = [[Client1_6]],
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
                    InstanceId = [[Client1_7]],
                    Class = [[Position]],
                    x = 2,
                    y = 0,
                    z = 0
                  }
                }
              },
              Position = {
                InstanceId = [[Client1_8]],
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
    Title = [[Npc chat grouped]]
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
      {
        InstanceId = [[Client1_730]],
        Class = [[TextManagerEntry]],
        Count = 1,
        Text = [[Stay close.]]
      }
    }
  },
  UserComponents = {
  },
  Versions = {
    Act = 3,
    ActivitySequence = 0,
    ActivityStep = 1,
    Behavior = 1,
    ChatAction = 0,
    ChatSequence = 0,
    ChatStep = 0,
    DefaultFeature = 0,
    Location = 1,
    LogicEntityBehavior = 1,
    MapDescription = 0,
    NpcCustom = 0,
    NpcGrpFeature = 1,
    Position = 0,
    Scenario = 2,
    TextManager = 0,
    TextManagerEntry = 0
  }
}
