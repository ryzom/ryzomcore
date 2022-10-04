---- Header
-- Version = '1'
-- Signature = 'DEV:5d0c4f0ff6fe82cabbe07c0870581378'
-- HeaderMD5 = '887c4bafe9afb796c6c5da25bcec802d'
-- BodyMD5 = 'a991ed604540bec7f7a2b3e66b529677'
-- Title = 'Bastosh, the Fyros cook.'
-- Name = 'Example_01_-_A_Hearty_Meal.r2'
-- ShortDescription = 'Come and try Bastosh's delicious creations.'
-- FirstLocation = 'uiR2_Deserts11'
-- RingPointLevel = 'a1:d3'
-- CreateBy = 'Ring(Nevrax)'
-- CreationDate = '10/03/06 16:52:00'
-- ModifiedBy = ''
-- ModificationDate = '10/03/06 16:52:00'
-- Rules = 'Masterless'
-- Level = '100'
-- Type = 'so_story_telling'
-- Language = 'en'
-- InitialIsland = 'uiR2_Deserts11'
-- InitialEntryPoint = 'uiR2EntryPoint05'
-- InitialSeason = 'Spring'
---- /Header

scenario = {
  InstanceId = [[Client1_109]],  
  Class = [[Scenario]],  
  AccessRules = [[liberal]],  
  InheritPos = 1,  
  Language = [[en]],  
  Name = [[Example_01_-_A_Hearty_Meal.r2]],  
  VersionName = [[PRE_RELEASE_@files]].."["..[[R:\output\r2_patch\ryzom_?????.idx]].."]"..[[]].."]"..[[]],  
  Acts = {
    {
      InstanceId = [[Client1_112]],  
      Class = [[Act]],  
      Version = 6,  
      InheritPos = 1,  
      LocationId = [[]],  
      ManualWeather = 0,  
      Name = [[Permanent]],  
      Season = 0,  
      ShortDescription = [[]],  
      Title = [[]],  
      WeatherValue = 0,  
      ActivitiesIds = {
      },  
      Behavior = {
        InstanceId = [[Client1_110]],  
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
          InstanceId = [[Client1_113]],  
          Class = [[DefaultFeature]],  
          Components = {
            {
              InstanceId = [[Client1_1814]],  
              Class = [[Npc]],  
              Angle = -0.25,  
              Base = [[palette.entities.botobjects.fy_s2_palmtree_e]],  
              InheritPos = 1,  
              Name = [[olansi V 3]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1812]],  
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
                InstanceId = [[Client1_1815]],  
                Class = [[Position]],  
                x = 29501.53125,  
                y = -1446.921875,  
                z = 75.96875
              }
            },  
            {
              InstanceId = [[Client1_1810]],  
              Class = [[Npc]],  
              Angle = -1.453125,  
              Base = [[palette.entities.botobjects.fy_s2_coconuts_a]],  
              InheritPos = 1,  
              Name = [[olash I 4]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1808]],  
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
                InstanceId = [[Client1_1811]],  
                Class = [[Position]],  
                x = 29525.95313,  
                y = -1475.25,  
                z = 74.21875
              }
            },  
            {
              InstanceId = [[Client1_1806]],  
              Class = [[Npc]],  
              Angle = 1.203125,  
              Base = [[palette.entities.botobjects.fy_s2_palmtree_c]],  
              InheritPos = 1,  
              Name = [[olansi III 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1804]],  
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
                InstanceId = [[Client1_1807]],  
                Class = [[Position]],  
                x = 29549.20313,  
                y = -1528.09375,  
                z = 76.625
              }
            },  
            {
              InstanceId = [[Client1_1802]],  
              Class = [[Npc]],  
              Angle = 1.78125,  
              Base = [[palette.entities.botobjects.fy_s2_coconuts_b]],  
              InheritPos = 1,  
              Name = [[olash II 4]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1800]],  
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
                InstanceId = [[Client1_1803]],  
                Class = [[Position]],  
                x = 29625.90625,  
                y = -1474.34375,  
                z = 78.15625
              }
            },  
            {
              InstanceId = [[Client1_1786]],  
              Class = [[Npc]],  
              Angle = -0.375,  
              Base = [[palette.entities.botobjects.fy_s2_palmtree_c]],  
              InheritPos = 1,  
              Name = [[olansi III 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1784]],  
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
                InstanceId = [[Client1_1787]],  
                Class = [[Position]],  
                x = 29505.9375,  
                y = -1551.5625,  
                z = 72.25
              }
            },  
            {
              InstanceId = [[Client1_1782]],  
              Class = [[Npc]],  
              Angle = 2.859375,  
              Base = [[palette.entities.botobjects.fy_s2_palmtree_a]],  
              InheritPos = 1,  
              Name = [[olansi I 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1780]],  
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
                InstanceId = [[Client1_1783]],  
                Class = [[Position]],  
                x = 29578.21875,  
                y = -1540.890625,  
                z = 74.796875
              }
            },  
            {
              InstanceId = [[Client1_1778]],  
              Class = [[Npc]],  
              Angle = 2.53125,  
              Base = [[palette.entities.botobjects.fy_s2_palmtree_d]],  
              InheritPos = 1,  
              Name = [[olansi IV 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1776]],  
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
                InstanceId = [[Client1_1779]],  
                Class = [[Position]],  
                x = 29620.4375,  
                y = -1496.703125,  
                z = 75.65625
              }
            },  
            {
              InstanceId = [[Client1_1774]],  
              Class = [[Npc]],  
              Angle = 0.90625,  
              Base = [[palette.entities.botobjects.fy_s2_palmtree_b]],  
              InheritPos = 1,  
              Name = [[olansi II 3]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1772]],  
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
                InstanceId = [[Client1_1775]],  
                Class = [[Position]],  
                x = 29498.98438,  
                y = -1484.09375,  
                z = 75.28125
              }
            },  
            {
              InstanceId = [[Client1_1770]],  
              Class = [[Npc]],  
              Angle = 1.125,  
              Base = [[palette.entities.botobjects.fy_s2_palmtree_d]],  
              InheritPos = 1,  
              Name = [[olansi IV 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1768]],  
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
                InstanceId = [[Client1_1771]],  
                Class = [[Position]],  
                x = 29558.89063,  
                y = -1471.953125,  
                z = 79.140625
              }
            },  
            {
              InstanceId = [[Client1_1758]],  
              Class = [[Npc]],  
              Angle = 1.09375,  
              Base = [[palette.entities.botobjects.fy_s2_coconuts_b]],  
              InheritPos = 1,  
              Name = [[olash II 3]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1756]],  
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
                InstanceId = [[Client1_1759]],  
                Class = [[Position]],  
                x = 29576.23438,  
                y = -1514.984375,  
                z = 73.125
              }
            },  
            {
              InstanceId = [[Client1_1754]],  
              Class = [[Npc]],  
              Angle = 0.546875,  
              Base = [[palette.entities.botobjects.fy_s2_coconuts_a]],  
              InheritPos = 1,  
              Name = [[olash I 3]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1752]],  
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
                InstanceId = [[Client1_1755]],  
                Class = [[Position]],  
                x = 29565.54688,  
                y = -1439.828125,  
                z = 75.015625
              }
            },  
            {
              InstanceId = [[Client1_1750]],  
              Class = [[Npc]],  
              Angle = 0.34375,  
              Base = [[palette.entities.botobjects.fy_s2_palmtree_b]],  
              InheritPos = 1,  
              Name = [[olansi II 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1748]],  
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
                InstanceId = [[Client1_1751]],  
                Class = [[Position]],  
                x = 29532.04688,  
                y = -1440.046875,  
                z = 75
              }
            },  
            {
              InstanceId = [[Client1_1746]],  
              Class = [[Npc]],  
              Angle = 1,  
              Base = [[palette.entities.botobjects.fy_s2_palmtree_e]],  
              InheritPos = 1,  
              Name = [[olansi V 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1744]],  
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
                InstanceId = [[Client1_1747]],  
                Class = [[Position]],  
                x = 29525.6875,  
                y = -1512.953125,  
                z = 77.90625
              }
            },  
            {
              InstanceId = [[Client1_1742]],  
              Class = [[Npc]],  
              Angle = 1.359375,  
              Base = [[palette.entities.botobjects.fy_s2_palmtree_b]],  
              InheritPos = 1,  
              Name = [[olansi II 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1740]],  
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
                InstanceId = [[Client1_1743]],  
                Class = [[Position]],  
                x = 29595.29688,  
                y = -1474.84375,  
                z = 74.796875
              }
            },  
            {
              InstanceId = [[Client1_1200]],  
              Class = [[Region]],  
              InheritPos = 1,  
              Name = [[Place 4]],  
              Points = {
                {
                  InstanceId = [[Client1_1202]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1203]],  
                    Class = [[Position]],  
                    x = 29852.95313,  
                    y = -1209.328125,  
                    z = 62.234375
                  }
                },  
                {
                  InstanceId = [[Client1_1205]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1206]],  
                    Class = [[Position]],  
                    x = 29874.92188,  
                    y = -1220.6875,  
                    z = 61.921875
                  }
                },  
                {
                  InstanceId = [[Client1_1208]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1209]],  
                    Class = [[Position]],  
                    x = 29882.78125,  
                    y = -1202.15625,  
                    z = 68.28125
                  }
                },  
                {
                  InstanceId = [[Client1_1211]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1212]],  
                    Class = [[Position]],  
                    x = 29869.75,  
                    y = -1176.25,  
                    z = 62.4375
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_1199]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_1163]],  
              Class = [[Region]],  
              InheritPos = 1,  
              Name = [[Place 3]],  
              Points = {
                {
                  InstanceId = [[Client1_1165]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1166]],  
                    Class = [[Position]],  
                    x = 29827.57813,  
                    y = -1226.765625,  
                    z = 65.1875
                  }
                },  
                {
                  InstanceId = [[Client1_1168]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1169]],  
                    Class = [[Position]],  
                    x = 29864.1875,  
                    y = -1244.71875,  
                    z = 64.828125
                  }
                },  
                {
                  InstanceId = [[Client1_1171]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1172]],  
                    Class = [[Position]],  
                    x = 29874.01563,  
                    y = -1222.375,  
                    z = 62.921875
                  }
                },  
                {
                  InstanceId = [[Client1_1174]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1175]],  
                    Class = [[Position]],  
                    x = 29836.89063,  
                    y = -1205.03125,  
                    z = 62.953125
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_1162]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_1126]],  
              Class = [[Region]],  
              InheritPos = 1,  
              Name = [[Place 2]],  
              Points = {
                {
                  InstanceId = [[Client1_1128]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1129]],  
                    Class = [[Position]],  
                    x = 29833.85938,  
                    y = -1173.890625,  
                    z = 61.6875
                  }
                },  
                {
                  InstanceId = [[Client1_1131]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1132]],  
                    Class = [[Position]],  
                    x = 29837.35938,  
                    y = -1203.4375,  
                    z = 61.59375
                  }
                },  
                {
                  InstanceId = [[Client1_1134]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1135]],  
                    Class = [[Position]],  
                    x = 29852.07813,  
                    y = -1209.25,  
                    z = 61.78125
                  }
                },  
                {
                  InstanceId = [[Client1_1137]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1138]],  
                    Class = [[Position]],  
                    x = 29869.25,  
                    y = -1174.640625,  
                    z = 62.484375
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_1125]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_1089]],  
              Class = [[Region]],  
              InheritPos = 1,  
              Name = [[Place 1]],  
              Points = {
                {
                  InstanceId = [[Client1_1091]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1092]],  
                    Class = [[Position]],  
                    x = 29814.8125,  
                    y = -1210.015625,  
                    z = 65.765625
                  }
                },  
                {
                  InstanceId = [[Client1_1094]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1095]],  
                    Class = [[Position]],  
                    x = 29826.51563,  
                    y = -1226.34375,  
                    z = 65.703125
                  }
                },  
                {
                  InstanceId = [[Client1_1097]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1098]],  
                    Class = [[Position]],  
                    x = 29836.57813,  
                    y = -1203.453125,  
                    z = 61.734375
                  }
                },  
                {
                  InstanceId = [[Client1_1100]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1101]],  
                    Class = [[Position]],  
                    x = 29833.40625,  
                    y = -1173.328125,  
                    z = 61.9375
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_1088]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_984]],  
              Class = [[Region]],  
              InheritPos = 1,  
              Name = [[Place Lyrius]],  
              Points = {
                {
                  InstanceId = [[Client1_986]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_987]],  
                    Class = [[Position]],  
                    x = 29780.60938,  
                    y = -1441.8125,  
                    z = 73.484375
                  }
                },  
                {
                  InstanceId = [[Client1_989]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_990]],  
                    Class = [[Position]],  
                    x = 29782.42188,  
                    y = -1412.703125,  
                    z = 74.890625
                  }
                },  
                {
                  InstanceId = [[Client1_992]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_993]],  
                    Class = [[Position]],  
                    x = 29750.54688,  
                    y = -1414.296875,  
                    z = 75.046875
                  }
                },  
                {
                  InstanceId = [[Client1_995]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_996]],  
                    Class = [[Position]],  
                    x = 29750.45313,  
                    y = -1438.3125,  
                    z = 75
                  }
                },  
                {
                  InstanceId = [[Client1_998]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_999]],  
                    Class = [[Position]],  
                    x = 29764.76563,  
                    y = -1463.609375,  
                    z = 73
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_983]],  
                Class = [[Position]],  
                x = 3.75,  
                y = 5,  
                z = 0.1875
              }
            },  
            {
              InstanceId = [[Client1_953]],  
              Class = [[Region]],  
              InheritPos = 1,  
              Name = [[Place Guedon]],  
              Points = {
                {
                  InstanceId = [[Client1_955]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_956]],  
                    Class = [[Position]],  
                    x = 29765.95313,  
                    y = -1461.34375,  
                    z = 73.53125
                  }
                },  
                {
                  InstanceId = [[Client1_958]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_959]],  
                    Class = [[Position]],  
                    x = 29732.5,  
                    y = -1457.546875,  
                    z = 74.96875
                  }
                },  
                {
                  InstanceId = [[Client1_961]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_962]],  
                    Class = [[Position]],  
                    x = 29726.73438,  
                    y = -1437.90625,  
                    z = 75.046875
                  }
                },  
                {
                  InstanceId = [[Client1_964]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_965]],  
                    Class = [[Position]],  
                    x = 29754.54688,  
                    y = -1431,  
                    z = 75.0625
                  }
                },  
                {
                  InstanceId = [[Client1_967]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_968]],  
                    Class = [[Position]],  
                    x = 29773.89063,  
                    y = -1448.9375,  
                    z = 73.828125
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_952]],  
                Class = [[Position]],  
                x = 2.921875,  
                y = -2.109375,  
                z = -0.1875
              }
            },  
            {
              InstanceId = [[Client1_933]],  
              Class = [[Region]],  
              InheritPos = 1,  
              Name = [[Place Dylion]],  
              Points = {
                {
                  InstanceId = [[Client1_935]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_936]],  
                    Class = [[Position]],  
                    x = 29782.125,  
                    y = -1426.578125,  
                    z = 74.953125
                  }
                },  
                {
                  InstanceId = [[Client1_938]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_939]],  
                    Class = [[Position]],  
                    x = 29777.5,  
                    y = -1391.671875,  
                    z = 74.703125
                  }
                },  
                {
                  InstanceId = [[Client1_941]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_942]],  
                    Class = [[Position]],  
                    x = 29760.85938,  
                    y = -1399.40625,  
                    z = 74.984375
                  }
                },  
                {
                  InstanceId = [[Client1_944]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_945]],  
                    Class = [[Position]],  
                    x = 29752.45313,  
                    y = -1421.859375,  
                    z = 75.34375
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_932]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_913]],  
              Class = [[Region]],  
              InheritPos = 1,  
              Name = [[Place Diorus]],  
              Points = {
                {
                  InstanceId = [[Client1_915]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_916]],  
                    Class = [[Position]],  
                    x = 29761.35938,  
                    y = -1383.375,  
                    z = 74.984375
                  }
                },  
                {
                  InstanceId = [[Client1_918]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_919]],  
                    Class = [[Position]],  
                    x = 29745.39063,  
                    y = -1407.015625,  
                    z = 76.8125
                  }
                },  
                {
                  InstanceId = [[Client1_921]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_922]],  
                    Class = [[Position]],  
                    x = 29772.20313,  
                    y = -1408.953125,  
                    z = 75
                  }
                },  
                {
                  InstanceId = [[Client1_924]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_925]],  
                    Class = [[Position]],  
                    x = 29777.57813,  
                    y = -1379.296875,  
                    z = 74.40625
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_912]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_871]],  
              Class = [[Region]],  
              InheritPos = 1,  
              Name = [[Place Xathus]],  
              Points = {
                {
                  InstanceId = [[Client1_873]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_874]],  
                    Class = [[Position]],  
                    x = 29716.54688,  
                    y = -1414.296875,  
                    z = 77.515625
                  }
                },  
                {
                  InstanceId = [[Client1_876]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_877]],  
                    Class = [[Position]],  
                    x = 29736.17188,  
                    y = -1400.09375,  
                    z = 79.515625
                  }
                },  
                {
                  InstanceId = [[Client1_879]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_880]],  
                    Class = [[Position]],  
                    x = 29755.29688,  
                    y = -1410.90625,  
                    z = 75.3125
                  }
                },  
                {
                  InstanceId = [[Client1_882]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_883]],  
                    Class = [[Position]],  
                    x = 29729.98438,  
                    y = -1435.046875,  
                    z = 75.203125
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_870]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_851]],  
              Class = [[Region]],  
              InheritPos = 1,  
              Name = [[Place Pirus]],  
              Points = {
                {
                  InstanceId = [[Client1_853]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_854]],  
                    Class = [[Position]],  
                    x = 29721.84375,  
                    y = -1412.65625,  
                    z = 78.59375
                  }
                },  
                {
                  InstanceId = [[Client1_856]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_857]],  
                    Class = [[Position]],  
                    x = 29706.75,  
                    y = -1429.890625,  
                    z = 74.890625
                  }
                },  
                {
                  InstanceId = [[Client1_859]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_860]],  
                    Class = [[Position]],  
                    x = 29734.375,  
                    y = -1438.546875,  
                    z = 75.015625
                  }
                },  
                {
                  InstanceId = [[Client1_862]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_863]],  
                    Class = [[Position]],  
                    x = 29751.125,  
                    y = -1417.5,  
                    z = 75.53125
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_850]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_825]],  
              Class = [[Region]],  
              InheritPos = 1,  
              Name = [[Place Ulydix]],  
              Points = {
                {
                  InstanceId = [[Client1_827]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_828]],  
                    Class = [[Position]],  
                    x = 29729.32813,  
                    y = -1453.40625,  
                    z = 75
                  }
                },  
                {
                  InstanceId = [[Client1_830]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_831]],  
                    Class = [[Position]],  
                    x = 29743.35938,  
                    y = -1457.625,  
                    z = 73.9375
                  }
                },  
                {
                  InstanceId = [[Client1_833]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_834]],  
                    Class = [[Position]],  
                    x = 29756.5625,  
                    y = -1438.40625,  
                    z = 74.984375
                  }
                },  
                {
                  InstanceId = [[Client1_836]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_837]],  
                    Class = [[Position]],  
                    x = 29743.375,  
                    y = -1427.203125,  
                    z = 74.703125
                  }
                },  
                {
                  InstanceId = [[Client1_844]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_845]],  
                    Class = [[Position]],  
                    x = 29723.85938,  
                    y = -1430.203125,  
                    z = 75.484375
                  }
                },  
                {
                  InstanceId = [[Client1_839]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_840]],  
                    Class = [[Position]],  
                    x = 29719.54688,  
                    y = -1446.734375,  
                    z = 75
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_824]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_675]],  
              Class = [[Npc]],  
              Angle = 3.03125,  
              Base = [[palette.entities.botobjects.fy_s1_baobab_a]],  
              InheritPos = 1,  
              Name = [[botoga I 9]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_673]],  
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
                InstanceId = [[Client1_676]],  
                Class = [[Position]],  
                x = 29612.40625,  
                y = -1026.5,  
                z = 75.015625
              }
            },  
            {
              InstanceId = [[Client1_671]],  
              Class = [[Npc]],  
              Angle = -0.5625,  
              Base = [[palette.entities.botobjects.fy_s1_baobab_b]],  
              InheritPos = 1,  
              Name = [[botoga II 7]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_669]],  
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
                InstanceId = [[Client1_672]],  
                Class = [[Position]],  
                x = 29605.75,  
                y = -1041.765625,  
                z = 75
              }
            },  
            {
              InstanceId = [[Client1_667]],  
              Class = [[Npc]],  
              Angle = -3.609375,  
              Base = [[palette.entities.botobjects.fy_s1_baobab_a]],  
              InheritPos = 1,  
              Name = [[botoga I 8]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_665]],  
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
                InstanceId = [[Client1_668]],  
                Class = [[Position]],  
                x = 29664.98438,  
                y = -1088.546875,  
                z = 73.015625
              }
            },  
            {
              InstanceId = [[Client1_663]],  
              Class = [[Npc]],  
              Angle = -1.0625,  
              Base = [[palette.entities.botobjects.fy_s1_baobab_a]],  
              InheritPos = 1,  
              Name = [[botoga I 7]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_661]],  
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
                InstanceId = [[Client1_664]],  
                Class = [[Position]],  
                x = 29638.28125,  
                y = -1104.578125,  
                z = 75.125
              }
            },  
            {
              InstanceId = [[Client1_659]],  
              Class = [[Npc]],  
              Angle = 1.625,  
              Base = [[palette.entities.botobjects.fy_s1_baobab_b]],  
              InheritPos = 1,  
              Name = [[botoga II 6]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_657]],  
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
                InstanceId = [[Client1_660]],  
                Class = [[Position]],  
                x = 29591.5,  
                y = -1077.28125,  
                z = 75.8125
              }
            },  
            {
              InstanceId = [[Client1_655]],  
              Class = [[Npc]],  
              Angle = 2.328125,  
              Base = [[palette.entities.botobjects.fy_s1_baobab_c]],  
              InheritPos = 1,  
              Name = [[botoga III 6]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_653]],  
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
                InstanceId = [[Client1_656]],  
                Class = [[Position]],  
                x = 29603.07813,  
                y = -1061.671875,  
                z = 75.015625
              }
            },  
            {
              InstanceId = [[Client1_4887]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Route Eunix]],  
              Points = {
                {
                  InstanceId = [[Client1_4889]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4890]],  
                    Class = [[Position]],  
                    x = 29777.85938,  
                    y = -1453.796875,  
                    z = 73
                  }
                },  
                {
                  InstanceId = [[Client1_4892]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4893]],  
                    Class = [[Position]],  
                    x = 29775.32813,  
                    y = -1453.296875,  
                    z = 73
                  }
                },  
                {
                  InstanceId = [[Client1_4895]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4896]],  
                    Class = [[Position]],  
                    x = 29772.875,  
                    y = -1454.40625,  
                    z = 73
                  }
                },  
                {
                  InstanceId = [[Client1_4898]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4899]],  
                    Class = [[Position]],  
                    x = 29770.8125,  
                    y = -1457.3125,  
                    z = 72.984375
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_4886]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_4869]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Route Boedix]],  
              Points = {
                {
                  InstanceId = [[Client1_4871]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4872]],  
                    Class = [[Position]],  
                    x = 29781.92188,  
                    y = -1450.046875,  
                    z = 72.953125
                  }
                },  
                {
                  InstanceId = [[Client1_4884]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4885]],  
                    Class = [[Position]],  
                    x = 29780.14063,  
                    y = -1449.1875,  
                    z = 73
                  }
                },  
                {
                  InstanceId = [[Client1_4874]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4875]],  
                    Class = [[Position]],  
                    x = 29777.29688,  
                    y = -1450.0625,  
                    z = 72.984375
                  }
                },  
                {
                  InstanceId = [[Client1_4881]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4882]],  
                    Class = [[Position]],  
                    x = 29774.8125,  
                    y = -1452.6875,  
                    z = 72.984375
                  }
                },  
                {
                  InstanceId = [[Client1_4877]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4878]],  
                    Class = [[Position]],  
                    x = 29770.5,  
                    y = -1459.390625,  
                    z = 73.65625
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_4868]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_4857]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Route Lyrius]],  
              Points = {
                {
                  InstanceId = [[Client1_4859]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4860]],  
                    Class = [[Position]],  
                    x = 29767.07813,  
                    y = -1454.640625,  
                    z = 73.828125
                  }
                },  
                {
                  InstanceId = [[Client1_4862]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4863]],  
                    Class = [[Position]],  
                    x = 29767.35938,  
                    y = -1457.265625,  
                    z = 72.984375
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_4856]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_4845]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Route Guedon]],  
              Points = {
                {
                  InstanceId = [[Client1_4847]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4848]],  
                    Class = [[Position]],  
                    x = 29764.0625,  
                    y = -1459.90625,  
                    z = 72.96875
                  }
                },  
                {
                  InstanceId = [[Client1_4850]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4851]],  
                    Class = [[Position]],  
                    x = 29765.95313,  
                    y = -1461.09375,  
                    z = 73.53125
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_4844]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_4834]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Route Ulydix]],  
              Points = {
                {
                  InstanceId = [[Client1_4836]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4837]],  
                    Class = [[Position]],  
                    x = 29758.90625,  
                    y = -1454.015625,  
                    z = 74
                  }
                },  
                {
                  InstanceId = [[Client1_4839]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4840]],  
                    Class = [[Position]],  
                    x = 29765.04688,  
                    y = -1459.40625,  
                    z = 72.921875
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_4833]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_4825]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Route Pirus]],  
              Points = {
                {
                  InstanceId = [[Client1_4827]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4828]],  
                    Class = [[Position]],  
                    x = 29759.98438,  
                    y = -1453.046875,  
                    z = 73
                  }
                },  
                {
                  InstanceId = [[Client1_4830]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4831]],  
                    Class = [[Position]],  
                    x = 29766.375,  
                    y = -1459.90625,  
                    z = 73
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_4824]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_4806]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Route Xathus]],  
              Points = {
                {
                  InstanceId = [[Client1_4808]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4809]],  
                    Class = [[Position]],  
                    x = 29762.10938,  
                    y = -1452.140625,  
                    z = 73.0625
                  }
                },  
                {
                  InstanceId = [[Client1_4811]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4812]],  
                    Class = [[Position]],  
                    x = 29765.85938,  
                    y = -1458.046875,  
                    z = 73.0625
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_4805]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_4391]],  
              Class = [[Region]],  
              InheritPos = 1,  
              Name = [[Place Armas]],  
              Points = {
                {
                  InstanceId = [[Client1_4393]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4394]],  
                    Class = [[Position]],  
                    x = 29776.625,  
                    y = -1461.625,  
                    z = 74.8125
                  }
                },  
                {
                  InstanceId = [[Client1_4399]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4400]],  
                    Class = [[Position]],  
                    x = 29782.59375,  
                    y = -1438.265625,  
                    z = 72.953125
                  }
                },  
                {
                  InstanceId = [[Client1_4402]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4403]],  
                    Class = [[Position]],  
                    x = 29777.71875,  
                    y = -1377.796875,  
                    z = 74.953125
                  }
                },  
                {
                  InstanceId = [[Client1_4405]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4406]],  
                    Class = [[Position]],  
                    x = 29761.35938,  
                    y = -1382.234375,  
                    z = 75
                  }
                },  
                {
                  InstanceId = [[Client1_4408]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4409]],  
                    Class = [[Position]],  
                    x = 29752.40625,  
                    y = -1398.796875,  
                    z = 76.140625
                  }
                },  
                {
                  InstanceId = [[Client1_4411]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4412]],  
                    Class = [[Position]],  
                    x = 29699,  
                    y = -1431.09375,  
                    z = 74.65625
                  }
                },  
                {
                  InstanceId = [[Client1_4414]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4415]],  
                    Class = [[Position]],  
                    x = 29709.75,  
                    y = -1450.515625,  
                    z = 74.96875
                  }
                },  
                {
                  InstanceId = [[Client1_4422]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4423]],  
                    Class = [[Position]],  
                    x = 29764.125,  
                    y = -1463.78125,  
                    z = 73
                  }
                },  
                {
                  InstanceId = [[Client1_4417]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4418]],  
                    Class = [[Position]],  
                    x = 29768.10938,  
                    y = -1459.875,  
                    z = 73.046875
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_4390]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_4164]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Route Diorus]],  
              Points = {
                {
                  InstanceId = [[Client1_4166]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4167]],  
                    Class = [[Position]],  
                    x = 29762.14063,  
                    y = -1434.5625,  
                    z = 74.984375
                  }
                },  
                {
                  InstanceId = [[Client1_4169]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4170]],  
                    Class = [[Position]],  
                    x = 29767.20313,  
                    y = -1458.9375,  
                    z = 73.046875
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_4163]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_4154]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Route Dylion]],  
              Points = {
                {
                  InstanceId = [[Client1_4156]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4157]],  
                    Class = [[Position]],  
                    x = 29769.03125,  
                    y = -1436.53125,  
                    z = 74.453125
                  }
                },  
                {
                  InstanceId = [[Client1_4159]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4160]],  
                    Class = [[Position]],  
                    x = 29768.01563,  
                    y = -1458.6875,  
                    z = 73
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_4153]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_4141]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Route Tu'Rlin]],  
              Points = {
                {
                  InstanceId = [[Client1_4146]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4147]],  
                    Class = [[Position]],  
                    x = 29773.96875,  
                    y = -1442.84375,  
                    z = 73.046875
                  }
                },  
                {
                  InstanceId = [[Client1_4149]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4150]],  
                    Class = [[Position]],  
                    x = 29769.4375,  
                    y = -1458.796875,  
                    z = 72.984375
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_4140]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_4121]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Route Zetonia]],  
              Points = {
                {
                  InstanceId = [[Client1_4123]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4124]],  
                    Class = [[Position]],  
                    x = 29776.82813,  
                    y = -1389.6875,  
                    z = 75.171875
                  }
                },  
                {
                  InstanceId = [[Client1_4126]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4127]],  
                    Class = [[Position]],  
                    x = 29769.28125,  
                    y = -1457.453125,  
                    z = 73.03125
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_4120]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_2295]],  
              Class = [[Npc]],  
              Angle = -0.03125,  
              Base = [[palette.entities.botobjects.fy_s2_lovejail_b]],  
              InheritPos = 1,  
              Name = [[loojine II 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2293]],  
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
                InstanceId = [[Client1_2296]],  
                Class = [[Position]],  
                x = 29523.79688,  
                y = -1504.4375,  
                z = 77.796875
              }
            },  
            {
              InstanceId = [[Client1_2291]],  
              Class = [[Npc]],  
              Angle = 2.03125,  
              Base = [[palette.entities.botobjects.fy_s2_lovejail_a]],  
              InheritPos = 1,  
              Name = [[loojine I 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2289]],  
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
                InstanceId = [[Client1_2292]],  
                Class = [[Position]],  
                x = 29548.29688,  
                y = -1510.84375,  
                z = 76.34375
              }
            },  
            {
              InstanceId = [[Client1_2287]],  
              Class = [[Npc]],  
              Angle = 1.015625,  
              Base = [[palette.entities.botobjects.fy_s2_lovejail_c]],  
              InheritPos = 1,  
              Name = [[loojine III 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2285]],  
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
                InstanceId = [[Client1_2288]],  
                Class = [[Position]],  
                x = 29529.23438,  
                y = -1526.875,  
                z = 76.046875
              }
            },  
            {
              InstanceId = [[Client1_2283]],  
              Class = [[Npc]],  
              Angle = 0.5,  
              Base = [[palette.entities.botobjects.fy_s2_lovejail_a]],  
              InheritPos = 1,  
              Name = [[loojine I 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2281]],  
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
                InstanceId = [[Client1_2284]],  
                Class = [[Position]],  
                x = 29542.78125,  
                y = -1494.6875,  
                z = 76.765625
              }
            },  
            {
              InstanceId = [[Client1_517]],  
              Class = [[Npc]],  
              Angle = -0.890625,  
              Base = [[palette.entities.botobjects.wind_turbine]],  
              InheritPos = 1,  
              Name = [[wind turbine 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_518]],  
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
                InstanceId = [[Client1_519]],  
                Class = [[Position]],  
                x = 29728.125,  
                y = -1394.28125,  
                z = 78.515625
              }
            },  
            {
              InstanceId = [[Client1_469]],  
              Class = [[Npc]],  
              Angle = 1.015625,  
              Base = [[palette.entities.botobjects.tent_fyros]],  
              InheritPos = 1,  
              Name = [[fyros tent 6]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_467]],  
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
                InstanceId = [[Client1_470]],  
                Class = [[Position]],  
                x = 29725.04688,  
                y = -1457.75,  
                z = 75.0625
              }
            },  
            {
              InstanceId = [[Client1_465]],  
              Class = [[Npc]],  
              Angle = -0.71875,  
              Base = [[palette.entities.botobjects.tent_fyros]],  
              InheritPos = 1,  
              Name = [[fyros tent 5]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_463]],  
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
                InstanceId = [[Client1_466]],  
                Class = [[Position]],  
                x = 29750.51563,  
                y = -1394.609375,  
                z = 75.53125
              }
            },  
            {
              InstanceId = [[Client1_430]],  
              Class = [[Region]],  
              InheritPos = 1,  
              Name = [[Place Bobo]],  
              Points = {
                {
                  InstanceId = [[Client1_432]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_433]],  
                    Class = [[Position]],  
                    x = 29783.48438,  
                    y = -1378.609375,  
                    z = 74.984375
                  }
                },  
                {
                  InstanceId = [[Client1_435]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_436]],  
                    Class = [[Position]],  
                    x = 29782.35938,  
                    y = -1378.453125,  
                    z = 74.984375
                  }
                },  
                {
                  InstanceId = [[Client1_438]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_439]],  
                    Class = [[Position]],  
                    x = 29780.89063,  
                    y = -1379.828125,  
                    z = 74.984375
                  }
                },  
                {
                  InstanceId = [[Client1_441]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_442]],  
                    Class = [[Position]],  
                    x = 29781.20313,  
                    y = -1381.65625,  
                    z = 74.984375
                  }
                },  
                {
                  InstanceId = [[Client1_444]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_445]],  
                    Class = [[Position]],  
                    x = 29782.59375,  
                    y = -1382.203125,  
                    z = 74.984375
                  }
                },  
                {
                  InstanceId = [[Client1_447]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_448]],  
                    Class = [[Position]],  
                    x = 29783.875,  
                    y = -1381.765625,  
                    z = 74.984375
                  }
                },  
                {
                  InstanceId = [[Client1_450]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_451]],  
                    Class = [[Position]],  
                    x = 29784.21875,  
                    y = -1380.171875,  
                    z = 74.984375
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_429]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_411]],  
              Class = [[Npc]],  
              Angle = 2.53125,  
              Base = [[palette.entities.botobjects.fy_s2_palmtree_e]],  
              InheritPos = 1,  
              Name = [[olansi V 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_409]],  
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
                InstanceId = [[Client1_412]],  
                Class = [[Position]],  
                x = 29785.01563,  
                y = -1391.375,  
                z = 74.671875
              }
            },  
            {
              InstanceId = [[Client1_407]],  
              Class = [[Npc]],  
              Angle = -2.875,  
              Base = [[palette.entities.botobjects.paddock]],  
              InheritPos = 1,  
              Name = [[paddock 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_405]],  
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
                InstanceId = [[Client1_408]],  
                Class = [[Position]],  
                x = 29782.53125,  
                y = -1380.015625,  
                z = 74.34375
              }
            },  
            {
              InstanceId = [[Client1_391]],  
              Class = [[Npc]],  
              Angle = 2.328125,  
              Base = [[palette.entities.botobjects.fy_s2_coconuts_b]],  
              InheritPos = 1,  
              Name = [[olash II 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_389]],  
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
                InstanceId = [[Client1_392]],  
                Class = [[Position]],  
                x = 29774.90625,  
                y = -1357.3125,  
                z = 74.046875
              }
            },  
            {
              InstanceId = [[Client1_387]],  
              Class = [[Npc]],  
              Angle = 1.875,  
              Base = [[palette.entities.botobjects.fy_s2_coconuts_a]],  
              InheritPos = 1,  
              Name = [[olash I 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_385]],  
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
                InstanceId = [[Client1_388]],  
                Class = [[Position]],  
                x = 29783.95313,  
                y = -1365.921875,  
                z = 74.25
              }
            },  
            {
              InstanceId = [[Client1_359]],  
              Class = [[Npc]],  
              Angle = 1.5625,  
              Base = [[palette.entities.botobjects.totem_pachyderm]],  
              InheritPos = 1,  
              Name = [[pachyderm totem 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_357]],  
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
                InstanceId = [[Client1_360]],  
                Class = [[Position]],  
                x = 29753.09375,  
                y = -1361.3125,  
                z = 74.046875
              }
            },  
            {
              InstanceId = [[Client1_341]],  
              Class = [[Npc]],  
              Angle = 1.765625,  
              Base = [[palette.entities.botobjects.totem_pachyderm]],  
              InheritPos = 1,  
              Name = [[pachyderm totem 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_339]],  
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
                InstanceId = [[Client1_342]],  
                Class = [[Position]],  
                x = 29768,  
                y = -1358.640625,  
                z = 74.578125
              }
            },  
            {
              InstanceId = [[Client1_337]],  
              Class = [[Npc]],  
              Angle = -2.921875,  
              Base = [[palette.entities.botobjects.merchant_RM_fyros]],  
              InheritPos = 1,  
              Name = [[fyros raw materials sign 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_335]],  
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
                InstanceId = [[Client1_338]],  
                Class = [[Position]],  
                x = 29782.29688,  
                y = -1436.828125,  
                z = 73.515625
              }
            },  
            {
              InstanceId = [[Client1_325]],  
              Class = [[Npc]],  
              Angle = -2.859375,  
              Base = [[palette.entities.botobjects.bag_b]],  
              InheritPos = 1,  
              Name = [[bag 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_323]],  
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
                InstanceId = [[Client1_326]],  
                Class = [[Position]],  
                x = 29787.67188,  
                y = -1432.25,  
                z = 74.09375
              }
            },  
            {
              InstanceId = [[Client1_321]],  
              Class = [[Npc]],  
              Angle = 2.15625,  
              Base = [[palette.entities.botobjects.pack_4]],  
              InheritPos = 1,  
              Name = [[pack 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_319]],  
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
                InstanceId = [[Client1_322]],  
                Class = [[Position]],  
                x = 29787.45313,  
                y = -1429.203125,  
                z = 74.203125
              }
            },  
            {
              InstanceId = [[Client1_317]],  
              Class = [[Npc]],  
              Angle = -2.03125,  
              Base = [[palette.entities.botobjects.pack_3]],  
              InheritPos = 1,  
              Name = [[pack 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_315]],  
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
                InstanceId = [[Client1_318]],  
                Class = [[Position]],  
                x = 29786.64063,  
                y = -1435.328125,  
                z = 73.828125
              }
            },  
            {
              InstanceId = [[Client1_301]],  
              Class = [[Npc]],  
              Angle = 0.984375,  
              Base = [[palette.entities.botobjects.fy_s2_coconuts_a]],  
              InheritPos = 1,  
              Name = [[olash I 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_299]],  
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
                InstanceId = [[Client1_302]],  
                Class = [[Position]],  
                x = 29700.48438,  
                y = -1464.5625,  
                z = 75.03125
              }
            },  
            {
              InstanceId = [[Client1_297]],  
              Class = [[Npc]],  
              Angle = -2.90625,  
              Base = [[palette.entities.botobjects.fy_s2_coconuts_b]],  
              InheritPos = 1,  
              Name = [[olash II 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_295]],  
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
                InstanceId = [[Client1_298]],  
                Class = [[Position]],  
                x = 29708.0625,  
                y = -1455.96875,  
                z = 75.0625
              }
            },  
            {
              InstanceId = [[Client1_293]],  
              Class = [[Npc]],  
              Angle = 3.484375,  
              Base = [[palette.entities.botobjects.totem_bird]],  
              InheritPos = 1,  
              Name = [[bird totem 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_291]],  
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
                InstanceId = [[Client1_294]],  
                Class = [[Position]],  
                x = 29685.04688,  
                y = -1434.859375,  
                z = 74.65625
              }
            },  
            {
              InstanceId = [[Client1_289]],  
              Class = [[Npc]],  
              Angle = -2.859375,  
              Base = [[palette.entities.botobjects.totem_bird]],  
              InheritPos = 1,  
              Name = [[bird totem 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_287]],  
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
                InstanceId = [[Client1_290]],  
                Class = [[Position]],  
                x = 29692.21875,  
                y = -1449.625,  
                z = 75
              }
            },  
            {
              InstanceId = [[Client1_249]],  
              Class = [[Npc]],  
              Angle = 1.984375,  
              Base = [[palette.entities.botobjects.merchant_focus_fyros]],  
              InheritPos = 1,  
              Name = [[fyros focus sign 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_247]],  
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
                InstanceId = [[Client1_250]],  
                Class = [[Position]],  
                x = 29770.04688,  
                y = -1463.671875,  
                z = 73.6875
              }
            },  
            {
              InstanceId = [[Client1_217]],  
              Class = [[Npc]],  
              Angle = -2.765625,  
              Base = [[palette.entities.botobjects.tent_fyros]],  
              InheritPos = 1,  
              Name = [[fyros tent 4]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_215]],  
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
                InstanceId = [[Client1_218]],  
                Class = [[Position]],  
                x = 29783.9375,  
                y = -1405.484375,  
                z = 74.984375
              }
            },  
            {
              InstanceId = [[Client1_213]],  
              Class = [[Npc]],  
              Angle = -0.5625,  
              Base = [[palette.entities.botobjects.tent_fyros]],  
              InheritPos = 1,  
              Name = [[fyros tent 3]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_211]],  
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
                InstanceId = [[Client1_214]],  
                Class = [[Position]],  
                x = 29710.79688,  
                y = -1418.453125,  
                z = 75.921875
              }
            },  
            {
              InstanceId = [[Client1_209]],  
              Class = [[Npc]],  
              Angle = -0.75,  
              Base = [[palette.entities.botobjects.tent_fyros]],  
              InheritPos = 1,  
              Name = [[fyros tent 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_207]],  
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
                InstanceId = [[Client1_210]],  
                Class = [[Position]],  
                x = 29725.21875,  
                y = -1404.34375,  
                z = 80.515625
              }
            },  
            {
              InstanceId = [[Client1_205]],  
              Class = [[Npc]],  
              Angle = 0.21875,  
              Base = [[palette.entities.botobjects.tent_cosmetics]],  
              InheritPos = 1,  
              Name = [[cosmetics tent 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_203]],  
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
                InstanceId = [[Client1_206]],  
                Class = [[Position]],  
                x = 29785.85938,  
                y = -1432.671875,  
                z = 73.859375
              }
            },  
            {
              InstanceId = [[Client1_201]],  
              Class = [[Npc]],  
              Angle = 1.375,  
              Base = [[palette.entities.botobjects.tent_fyros]],  
              InheritPos = 1,  
              Name = [[fyros tent 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_199]],  
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
                InstanceId = [[Client1_202]],  
                Class = [[Position]],  
                x = 29747.09375,  
                y = -1464.484375,  
                z = 74.046875
              }
            },  
            {
              InstanceId = [[Client1_197]],  
              Class = [[Npc]],  
              Angle = 1.4375,  
              Base = [[palette.entities.botobjects.chariot_working]],  
              InheritPos = 1,  
              Name = [[working chariot 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_195]],  
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
                InstanceId = [[Client1_198]],  
                Class = [[Position]],  
                x = 29767.60938,  
                y = -1467.65625,  
                z = 73.9375
              }
            },  
            {
              InstanceId = [[Client1_185]],  
              Class = [[Npc]],  
              Angle = 0.40625,  
              Base = [[palette.entities.botobjects.fx_fo_bugsa]],  
              InheritPos = 1,  
              Name = [[grey bugs 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_183]],  
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
                InstanceId = [[Client1_186]],  
                Class = [[Position]],  
                x = 29775.5,  
                y = -1468.875,  
                z = 74.40625
              }
            },  
            {
              InstanceId = [[Client1_173]],  
              Class = [[Npc]],  
              Angle = -1.453125,  
              Base = [[palette.entities.botobjects.carrion_mammal]],  
              InheritPos = 1,  
              Name = [[mammal carrion 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_171]],  
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
                InstanceId = [[Client1_174]],  
                Class = [[Position]],  
                x = 29775.54688,  
                y = -1469.03125,  
                z = 74.421875
              }
            },  
            {
              InstanceId = [[Client1_161]],  
              Class = [[Npc]],  
              Angle = 2,  
              Base = [[palette.entities.botobjects.barrel1]],  
              InheritPos = 1,  
              Name = [[barrel 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_159]],  
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
                InstanceId = [[Client1_162]],  
                Class = [[Position]],  
                x = 29773.39063,  
                y = -1464.234375,  
                z = 73.90625
              }
            },  
            {
              InstanceId = [[Client1_157]],  
              Class = [[Npc]],  
              Angle = 1.125,  
              Base = [[palette.entities.botobjects.jar_3]],  
              InheritPos = 1,  
              Name = [[3 jars 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_155]],  
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
                InstanceId = [[Client1_158]],  
                Class = [[Position]],  
                x = 29772.34375,  
                y = -1463.96875,  
                z = 73.8125
              }
            },  
            {
              InstanceId = [[Client1_153]],  
              Class = [[Npc]],  
              Angle = 2.0625,  
              Base = [[palette.entities.botobjects.fire_base]],  
              InheritPos = 1,  
              Name = [[fire base 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_151]],  
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
                InstanceId = [[Client1_154]],  
                Class = [[Position]],  
                x = 29768.6875,  
                y = -1461.046875,  
                z = 73.625
              }
            },  
            {
              InstanceId = [[Client1_149]],  
              Class = [[Npc]],  
              Angle = 2,  
              Base = [[palette.entities.botobjects.tent]],  
              InheritPos = 1,  
              Name = [[tent 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_147]],  
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
                InstanceId = [[Client1_150]],  
                Class = [[Position]],  
                x = 29771.71875,  
                y = -1467.234375,  
                z = 74.03125
              }
            },  
            {
              InstanceId = [[Client1_651]],  
              Class = [[Npc]],  
              Angle = 0.5,  
              Base = [[palette.entities.botobjects.fy_s1_baobab_a]],  
              InheritPos = 1,  
              Name = [[botoga I 6]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_649]],  
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
                InstanceId = [[Client1_652]],  
                Class = [[Position]],  
                x = 29628.59375,  
                y = -1039.953125,  
                z = 75.015625
              }
            },  
            {
              InstanceId = [[Client1_647]],  
              Class = [[Npc]],  
              Angle = 2.8125,  
              Base = [[palette.entities.botobjects.fy_s1_baobab_a]],  
              InheritPos = 1,  
              Name = [[botoga I 5]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_645]],  
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
                InstanceId = [[Client1_648]],  
                Class = [[Position]],  
                x = 29646.4375,  
                y = -1051.109375,  
                z = 75.484375
              }
            },  
            {
              InstanceId = [[Client1_643]],  
              Class = [[Npc]],  
              Angle = 2.25,  
              Base = [[palette.entities.botobjects.fy_s1_baobab_b]],  
              InheritPos = 1,  
              Name = [[botoga II 5]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_641]],  
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
                InstanceId = [[Client1_644]],  
                Class = [[Position]],  
                x = 29630.96875,  
                y = -1066.53125,  
                z = 76.59375
              }
            },  
            {
              InstanceId = [[Client1_639]],  
              Class = [[Npc]],  
              Angle = 2.40625,  
              Base = [[palette.entities.botobjects.fy_s1_baobab_a]],  
              InheritPos = 1,  
              Name = [[botoga I 4]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_637]],  
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
                InstanceId = [[Client1_640]],  
                Class = [[Position]],  
                x = 29597.48438,  
                y = -1103.34375,  
                z = 74.984375
              }
            },  
            {
              InstanceId = [[Client1_635]],  
              Class = [[Npc]],  
              Angle = 3.125,  
              Base = [[palette.entities.botobjects.fy_s1_baobab_c]],  
              InheritPos = 1,  
              Name = [[botoga III 5]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_633]],  
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
                InstanceId = [[Client1_636]],  
                Class = [[Position]],  
                x = 29616.01563,  
                y = -1078.515625,  
                z = 76.859375
              }
            },  
            {
              InstanceId = [[Client1_631]],  
              Class = [[Npc]],  
              Angle = -1.65625,  
              Base = [[palette.entities.botobjects.fy_s1_baobab_b]],  
              InheritPos = 1,  
              Name = [[botoga II 4]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_629]],  
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
                InstanceId = [[Client1_632]],  
                Class = [[Position]],  
                x = 29619.64063,  
                y = -1098.40625,  
                z = 75.421875
              }
            },  
            {
              InstanceId = [[Client1_627]],  
              Class = [[Npc]],  
              Angle = -2.046875,  
              Base = [[palette.entities.botobjects.fy_s1_baobab_c]],  
              InheritPos = 1,  
              Name = [[botoga III 4]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_625]],  
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
                InstanceId = [[Client1_628]],  
                Class = [[Position]],  
                x = 29613.75,  
                y = -1124.359375,  
                z = 74.890625
              }
            },  
            {
              InstanceId = [[Client1_623]],  
              Class = [[Npc]],  
              Angle = -1.390625,  
              Base = [[palette.entities.botobjects.fy_s1_baobab_b]],  
              InheritPos = 1,  
              Name = [[botoga II 3]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_621]],  
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
                InstanceId = [[Client1_624]],  
                Class = [[Position]],  
                x = 29627.70313,  
                y = -1137.546875,  
                z = 73.328125
              }
            },  
            {
              InstanceId = [[Client1_619]],  
              Class = [[Npc]],  
              Angle = -5.0625,  
              Base = [[palette.entities.botobjects.fy_s1_baobab_a]],  
              InheritPos = 1,  
              Name = [[botoga I 3]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_617]],  
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
                InstanceId = [[Client1_620]],  
                Class = [[Position]],  
                x = 29666.78125,  
                y = -1133.515625,  
                z = 72.75
              }
            },  
            {
              InstanceId = [[Client1_615]],  
              Class = [[Npc]],  
              Angle = -4.125,  
              Base = [[palette.entities.botobjects.fy_s1_baobab_b]],  
              InheritPos = 1,  
              Name = [[botoga II 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_613]],  
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
                InstanceId = [[Client1_616]],  
                Class = [[Position]],  
                x = 29629.51563,  
                y = -1119.03125,  
                z = 75
              }
            },  
            {
              InstanceId = [[Client1_611]],  
              Class = [[Npc]],  
              Angle = -2.625,  
              Base = [[palette.entities.botobjects.fy_s1_baobab_c]],  
              InheritPos = 1,  
              Name = [[botoga III 3]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_609]],  
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
                InstanceId = [[Client1_612]],  
                Class = [[Position]],  
                x = 29655.40625,  
                y = -1072.140625,  
                z = 75.140625
              }
            },  
            {
              InstanceId = [[Client1_607]],  
              Class = [[Npc]],  
              Angle = -3.09375,  
              Base = [[palette.entities.botobjects.fy_s1_baobab_a]],  
              InheritPos = 1,  
              Name = [[botoga I 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_605]],  
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
                InstanceId = [[Client1_608]],  
                Class = [[Position]],  
                x = 29645.48438,  
                y = -1088.59375,  
                z = 75.390625
              }
            },  
            {
              InstanceId = [[Client1_603]],  
              Class = [[Npc]],  
              Angle = -2.34375,  
              Base = [[palette.entities.botobjects.fy_s1_baobab_b]],  
              InheritPos = 1,  
              Name = [[botoga II 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_601]],  
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
                InstanceId = [[Client1_604]],  
                Class = [[Position]],  
                x = 29653.23438,  
                y = -1108.328125,  
                z = 74.59375
              }
            },  
            {
              InstanceId = [[Client1_599]],  
              Class = [[Npc]],  
              Angle = 2.984375,  
              Base = [[palette.entities.botobjects.fy_s1_baobab_c]],  
              InheritPos = 1,  
              Name = [[botoga III 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_597]],  
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
                InstanceId = [[Client1_600]],  
                Class = [[Position]],  
                x = 29647.625,  
                y = -1132.65625,  
                z = 73.5625
              }
            },  
            {
              InstanceId = [[Client1_595]],  
              Class = [[Npc]],  
              Angle = -1.25,  
              Base = [[palette.entities.botobjects.fy_s1_baobab_a]],  
              InheritPos = 1,  
              Name = [[botoga I 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_593]],  
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
                InstanceId = [[Client1_596]],  
                Class = [[Position]],  
                x = 29678.9375,  
                y = -1116.015625,  
                z = 74.953125
              }
            },  
            {
              InstanceId = [[Client1_591]],  
              Class = [[Npc]],  
              Angle = -2,  
              Base = [[palette.entities.botobjects.fy_s1_baobab_c]],  
              InheritPos = 1,  
              Name = [[botoga III 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_589]],  
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
                InstanceId = [[Client1_592]],  
                Class = [[Position]],  
                x = 29680.26563,  
                y = -1094.4375,  
                z = 74.109375
              }
            },  
            {
              InstanceId = [[Client1_587]],  
              Class = [[Npc]],  
              Angle = 1.3125,  
              Base = [[palette.entities.botobjects.crate1]],  
              InheritPos = 1,  
              Name = [[crate 11]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_585]],  
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
                InstanceId = [[Client1_588]],  
                Class = [[Position]],  
                x = 29743.46875,  
                y = -1446.75,  
                z = 74.84375
              }
            },  
            {
              InstanceId = [[Client1_583]],  
              Class = [[Npc]],  
              Angle = 2.734375,  
              Base = [[palette.entities.botobjects.crate1]],  
              InheritPos = 1,  
              Name = [[crate 10]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_581]],  
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
                InstanceId = [[Client1_584]],  
                Class = [[Position]],  
                x = 29780.03125,  
                y = -1455.796875,  
                z = 74.109375
              }
            },  
            {
              InstanceId = [[Client1_579]],  
              Class = [[Npc]],  
              Angle = 2.46875,  
              Base = [[palette.entities.botobjects.crate1]],  
              InheritPos = 1,  
              Name = [[crate 9]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_577]],  
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
                InstanceId = [[Client1_580]],  
                Class = [[Position]],  
                x = 29783.92188,  
                y = -1454.34375,  
                z = 74.453125
              }
            },  
            {
              InstanceId = [[Client1_575]],  
              Class = [[Npc]],  
              Angle = 3.328125,  
              Base = [[palette.entities.botobjects.crate1]],  
              InheritPos = 1,  
              Name = [[crate 8]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_573]],  
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
                InstanceId = [[Client1_576]],  
                Class = [[Position]],  
                x = 29785.03125,  
                y = -1450.484375,  
                z = 74.203125
              }
            },  
            {
              InstanceId = [[Client1_571]],  
              Class = [[Npc]],  
              Angle = -2.125,  
              Base = [[palette.entities.botobjects.crate1]],  
              InheritPos = 1,  
              Name = [[crate 7]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_569]],  
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
                InstanceId = [[Client1_572]],  
                Class = [[Position]],  
                x = 29736.92188,  
                y = -1419.046875,  
                z = 77.34375
              }
            },  
            {
              InstanceId = [[Client1_567]],  
              Class = [[Npc]],  
              Angle = 0.421875,  
              Base = [[palette.entities.botobjects.crate1]],  
              InheritPos = 1,  
              Name = [[crate 6]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_565]],  
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
                InstanceId = [[Client1_568]],  
                Class = [[Position]],  
                x = 29732.21875,  
                y = -1419.03125,  
                z = 77.671875
              }
            },  
            {
              InstanceId = [[Client1_563]],  
              Class = [[Npc]],  
              Angle = 0.015625,  
              Base = [[palette.entities.botobjects.crate1]],  
              InheritPos = 1,  
              Name = [[crate 5]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_561]],  
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
                InstanceId = [[Client1_564]],  
                Class = [[Position]],  
                x = 29729.53125,  
                y = -1422.671875,  
                z = 76.9375
              }
            },  
            {
              InstanceId = [[Client1_559]],  
              Class = [[Npc]],  
              Angle = -3.78125,  
              Base = [[palette.entities.botobjects.crate1]],  
              InheritPos = 1,  
              Name = [[crate 4]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_557]],  
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
                InstanceId = [[Client1_560]],  
                Class = [[Position]],  
                x = 29778.3125,  
                y = -1422.609375,  
                z = 74.390625
              }
            },  
            {
              InstanceId = [[Client1_555]],  
              Class = [[Npc]],  
              Angle = -2.109375,  
              Base = [[palette.entities.botobjects.crate1]],  
              InheritPos = 1,  
              Name = [[crate 3]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_553]],  
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
                InstanceId = [[Client1_556]],  
                Class = [[Position]],  
                x = 29776.79688,  
                y = -1416.71875,  
                z = 74.75
              }
            },  
            {
              InstanceId = [[Client1_551]],  
              Class = [[Npc]],  
              Angle = -2.609375,  
              Base = [[palette.entities.botobjects.crate1]],  
              InheritPos = 1,  
              Name = [[crate 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_549]],  
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
                InstanceId = [[Client1_552]],  
                Class = [[Position]],  
                x = 29735.1875,  
                y = -1447.390625,  
                z = 74.953125
              }
            },  
            {
              InstanceId = [[Client1_547]],  
              Class = [[Npc]],  
              Angle = 1.5625,  
              Base = [[palette.entities.botobjects.crate1]],  
              InheritPos = 1,  
              Name = [[crate 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_545]],  
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
                InstanceId = [[Client1_548]],  
                Class = [[Position]],  
                x = 29739.34375,  
                y = -1449.796875,  
                z = 74.859375
              }
            },  
            {
              InstanceId = [[Client1_543]],  
              Class = [[Npc]],  
              Angle = -2.671875,  
              Base = [[palette.entities.botobjects.campfire]],  
              InheritPos = 1,  
              Name = [[camp fire 4]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_541]],  
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
                InstanceId = [[Client1_544]],  
                Class = [[Position]],  
                x = 29774.70313,  
                y = -1420.296875,  
                z = 74.625
              }
            },  
            {
              InstanceId = [[Client1_539]],  
              Class = [[Npc]],  
              Angle = 1.265625,  
              Base = [[palette.entities.botobjects.campfire]],  
              InheritPos = 1,  
              Name = [[camp fire 3]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_537]],  
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
                InstanceId = [[Client1_540]],  
                Class = [[Position]],  
                x = 29739.51563,  
                y = -1445.734375,  
                z = 74.9375
              }
            },  
            {
              InstanceId = [[Client1_535]],  
              Class = [[Npc]],  
              Angle = 2.46875,  
              Base = [[palette.entities.botobjects.campfire]],  
              InheritPos = 1,  
              Name = [[camp fire 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_533]],  
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
                InstanceId = [[Client1_536]],  
                Class = [[Position]],  
                x = 29780.95313,  
                y = -1451.6875,  
                z = 73.890625
              }
            },  
            {
              InstanceId = [[Client1_527]],  
              Class = [[Npc]],  
              Angle = 0.03125,  
              Base = [[palette.entities.botobjects.campfire]],  
              InheritPos = 1,  
              Name = [[camp fire 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_525]],  
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
                InstanceId = [[Client1_528]],  
                Class = [[Position]],  
                x = 29733.9375,  
                y = -1422.234375,  
                z = 76.953125
              }
            },  
            {
              InstanceId = [[Client1_522]],  
              Class = [[Npc]],  
              Angle = -0.890625,  
              Base = [[palette.entities.botobjects.wind_turbine]],  
              InheritPos = 1,  
              Name = [[wind turbine 3]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_523]],  
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
                InstanceId = [[Client1_524]],  
                Class = [[Position]],  
                x = 29713.375,  
                y = -1392.359375,  
                z = 76.703125
              }
            },  
            {
              InstanceId = [[Client1_22288]],  
              Class = [[Npc]],  
              Angle = 0.9375,  
              Base = [[palette.entities.botobjects.fy_s2_palmtree_b]],  
              EntityCategory = [[Desert Plants]],  
              InheritPos = 1,  
              Name = [[olansi II 4]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_22286]],  
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
                InstanceId = [[Client1_22289]],  
                Class = [[Position]],  
                x = 29488.48438,  
                y = -1531.5625,  
                z = 74.109375
              }
            },  
            {
              InstanceId = [[Client1_22292]],  
              Class = [[Npc]],  
              Angle = 0.390625,  
              Base = [[palette.entities.botobjects.fy_s2_coconuts_b]],  
              EntityCategory = [[Desert Plants]],  
              InheritPos = 1,  
              Name = [[olash II 5]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_22290]],  
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
                InstanceId = [[Client1_22293]],  
                Class = [[Position]],  
                x = 29503.21875,  
                y = -1505.78125,  
                z = 75.03125
              }
            },  
            {
              InstanceId = [[Client1_22296]],  
              Class = [[Npc]],  
              Angle = 2.078125,  
              Base = [[palette.entities.botobjects.fy_s2_papaleaf_a]],  
              EntityCategory = [[Desert Plants]],  
              InheritPos = 1,  
              Name = [[papalexi I 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_22294]],  
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
                InstanceId = [[Client1_22297]],  
                Class = [[Position]],  
                x = 29605.6875,  
                y = -1519.71875,  
                z = 74.890625
              }
            },  
            {
              InstanceId = [[Client1_22300]],  
              Class = [[Npc]],  
              Angle = 0.140625,  
              Base = [[palette.entities.botobjects.fy_s2_papaleaf_b]],  
              EntityCategory = [[Desert Plants]],  
              InheritPos = 1,  
              Name = [[papalexi II 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_22298]],  
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
                InstanceId = [[Client1_22301]],  
                Class = [[Position]],  
                x = 29575.73438,  
                y = -1490.25,  
                z = 73.046875
              }
            },  
            {
              InstanceId = [[Client1_22304]],  
              Class = [[Npc]],  
              Angle = 0.296875,  
              Base = [[palette.entities.botobjects.fy_s2_papaleaf_a]],  
              EntityCategory = [[Desert Plants]],  
              InheritPos = 1,  
              Name = [[papalexi I 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_22302]],  
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
                InstanceId = [[Client1_22305]],  
                Class = [[Position]],  
                x = 29519.34375,  
                y = -1467.046875,  
                z = 74.828125
              }
            },  
            {
              InstanceId = [[Client1_22308]],  
              Class = [[Npc]],  
              Angle = 1.359375,  
              Base = [[palette.entities.botobjects.fy_s2_coconuts_a]],  
              EntityCategory = [[Desert Plants]],  
              InheritPos = 1,  
              Name = [[olash I 5]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_22306]],  
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
                InstanceId = [[Client1_22309]],  
                Class = [[Position]],  
                x = 29537.71875,  
                y = -1549.703125,  
                z = 74.890625
              }
            },  
            {
              InstanceId = [[Client1_22336]],  
              Class = [[Npc]],  
              Angle = 0.734375,  
              Base = [[palette.entities.botobjects.fy_s2_savantree_a]],  
              EntityCategory = [[Desert Plants]],  
              InheritPos = 1,  
              Name = [[savaniel I 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_22334]],  
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
                InstanceId = [[Client1_22337]],  
                Class = [[Position]],  
                x = 29509.20313,  
                y = -1526.625,  
                z = 76.34375
              }
            },  
            {
              InstanceId = [[Client1_22343]],  
              Class = [[Region]],  
              InheritPos = 1,  
              Name = [[Place Goaris]],  
              Points = {
                {
                  InstanceId = [[Client1_22345]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_22346]],  
                    Class = [[Position]],  
                    x = 29483.03125,  
                    y = -1555.796875,  
                    z = 74.875
                  }
                },  
                {
                  InstanceId = [[Client1_22348]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_22349]],  
                    Class = [[Position]],  
                    x = 29615.42188,  
                    y = -1565.484375,  
                    z = 74.015625
                  }
                },  
                {
                  InstanceId = [[Client1_22351]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_22352]],  
                    Class = [[Position]],  
                    x = 29626.54688,  
                    y = -1537.421875,  
                    z = 74.90625
                  }
                },  
                {
                  InstanceId = [[Client1_22354]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_22355]],  
                    Class = [[Position]],  
                    x = 29655.82813,  
                    y = -1529.359375,  
                    z = 75.09375
                  }
                },  
                {
                  InstanceId = [[Client1_22357]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_22358]],  
                    Class = [[Position]],  
                    x = 29661.1875,  
                    y = -1503.421875,  
                    z = 74.984375
                  }
                },  
                {
                  InstanceId = [[Client1_22360]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_22361]],  
                    Class = [[Position]],  
                    x = 29631.35938,  
                    y = -1459.546875,  
                    z = 76.625
                  }
                },  
                {
                  InstanceId = [[Client1_22363]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_22364]],  
                    Class = [[Position]],  
                    x = 29537.625,  
                    y = -1377.75,  
                    z = 74.796875
                  }
                },  
                {
                  InstanceId = [[Client1_22366]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_22367]],  
                    Class = [[Position]],  
                    x = 29512.15625,  
                    y = -1382.84375,  
                    z = 75.03125
                  }
                },  
                {
                  InstanceId = [[Client1_22369]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_22370]],  
                    Class = [[Position]],  
                    x = 29504.3125,  
                    y = -1411.78125,  
                    z = 74.875
                  }
                },  
                {
                  InstanceId = [[Client1_22372]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_22373]],  
                    Class = [[Position]],  
                    x = 29476.70313,  
                    y = -1422.671875,  
                    z = 73.9375
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_22342]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_22376]],  
              Class = [[Npc]],  
              Angle = -0.015625,  
              Base = [[palette.entities.botobjects.fy_s2_palmtree_e]],  
              EntityCategory = [[Desert Plants]],  
              InheritPos = 1,  
              Name = [[olansi V 4]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_22374]],  
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
                InstanceId = [[Client1_22377]],  
                Class = [[Position]],  
                x = 29539.75,  
                y = -1411.296875,  
                z = 75.265625
              }
            },  
            {
              InstanceId = [[Client1_22623]],  
              Class = [[Region]],  
              InheritPos = 1,  
              Name = [[Place Goari nid]],  
              Points = {
                {
                  InstanceId = [[Client1_22625]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_22626]],  
                    Class = [[Position]],  
                    x = 29505.98438,  
                    y = -1543.53125,  
                    z = 74.578125
                  }
                },  
                {
                  InstanceId = [[Client1_22628]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_22629]],  
                    Class = [[Position]],  
                    x = 29522.01563,  
                    y = -1541.09375,  
                    z = 75.0625
                  }
                },  
                {
                  InstanceId = [[Client1_22631]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_22632]],  
                    Class = [[Position]],  
                    x = 29528.625,  
                    y = -1532.03125,  
                    z = 75.3125
                  }
                },  
                {
                  InstanceId = [[Client1_22634]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_22635]],  
                    Class = [[Position]],  
                    x = 29525.5,  
                    y = -1515.65625,  
                    z = 77.21875
                  }
                },  
                {
                  InstanceId = [[Client1_22637]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_22638]],  
                    Class = [[Position]],  
                    x = 29511.20313,  
                    y = -1508,  
                    z = 78.1875
                  }
                },  
                {
                  InstanceId = [[Client1_22640]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_22641]],  
                    Class = [[Position]],  
                    x = 29500.40625,  
                    y = -1508.6875,  
                    z = 74.96875
                  }
                },  
                {
                  InstanceId = [[Client1_22643]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_22644]],  
                    Class = [[Position]],  
                    x = 29492.75,  
                    y = -1514.609375,  
                    z = 75.109375
                  }
                },  
                {
                  InstanceId = [[Client1_22646]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_22647]],  
                    Class = [[Position]],  
                    x = 29489.60938,  
                    y = -1524.015625,  
                    z = 74.4375
                  }
                },  
                {
                  InstanceId = [[Client1_22649]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_22650]],  
                    Class = [[Position]],  
                    x = 29493.09375,  
                    y = -1535.171875,  
                    z = 74.796875
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_22622]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            }
          }
        }
      },  
      Position = {
        InstanceId = [[Client1_111]],  
        Class = [[Position]],  
        x = 0,  
        y = 0,  
        z = 0
      }
    },  
    {
      InstanceId = [[Client1_116]],  
      Class = [[Act]],  
      Version = 6,  
      InheritPos = 1,  
      LocationId = [[Client1_118]],  
      ManualWeather = 1,  
      Name = [[Act 1]],  
      Season = 0,  
      ShortDescription = [[]],  
      Title = [[]],  
      WeatherValue = 0,  
      ActivitiesIds = {
      },  
      Behavior = {
        InstanceId = [[Client1_114]],  
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
          InstanceId = [[Client1_117]],  
          Class = [[DefaultFeature]],  
          Components = {
            {
              InstanceId = [[Client1_910]],  
              Class = [[NpcCustom]],  
              Angle = -0.75,  
              ArmColor = 3,  
              ArmModel = 5606190,  
              Base = [[palette.entities.npcs.civils.f_civil_70]],  
              EyesColor = 4,  
              FeetColor = 1,  
              FeetModel = 5605422,  
              GabaritArmsWidth = 7,  
              GabaritBreastSize = 8,  
              GabaritHeight = 9,  
              GabaritLegsWidth = 9,  
              GabaritTorsoWidth = 5,  
              HairColor = 4,  
              HairType = 5621550,  
              HandsColor = 1,  
              HandsModel = 0,  
              InheritPos = 1,  
              JacketColor = 4,  
              JacketModel = 6704430,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 4,  
              MorphTarget2 = 2,  
              MorphTarget3 = 0,  
              MorphTarget4 = 0,  
              MorphTarget5 = 6,  
              MorphTarget6 = 7,  
              MorphTarget7 = 3,  
              MorphTarget8 = 5,  
              Name = [[Diorus]],  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_civil_light_melee_blunt_c2.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 0,  
              Tattoo = 22,  
              TrouserColor = 5,  
              TrouserModel = 6703406,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_908]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_3010]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_3014]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_3006]]),  
                        Action = {
                          InstanceId = [[Client1_3013]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_3011]],  
                      Class = [[EventType]],  
                      Type = [[targeted by player]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_926]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_927]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Wander]],  
                        ActivityZoneId = r2.RefId([[Client1_913]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_4162]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_4171]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_4164]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_911]],  
                Class = [[Position]],  
                x = 29755.26563,  
                y = -1399.15625,  
                z = 75.296875
              }
            },  
            {
              InstanceId = [[Client1_930]],  
              Class = [[NpcCustom]],  
              Angle = -2.71875,  
              ArmColor = 5,  
              ArmModel = 5606190,  
              Base = [[palette.entities.npcs.civils.f_civil_70]],  
              EyesColor = 4,  
              FeetColor = 5,  
              FeetModel = 6702382,  
              GabaritArmsWidth = 12,  
              GabaritBreastSize = 12,  
              GabaritHeight = 14,  
              GabaritLegsWidth = 9,  
              GabaritTorsoWidth = 3,  
              HairColor = 3,  
              HairType = 2606,  
              HandsColor = 3,  
              HandsModel = 6702894,  
              InheritPos = 1,  
              JacketColor = 1,  
              JacketModel = 6704430,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 5,  
              MorphTarget2 = 4,  
              MorphTarget3 = 1,  
              MorphTarget4 = 6,  
              MorphTarget5 = 4,  
              MorphTarget6 = 4,  
              MorphTarget7 = 4,  
              MorphTarget8 = 4,  
              Name = [[Dylion]],  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_civil_light_melee_blunt_c2.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 0,  
              Tattoo = 20,  
              TrouserColor = 0,  
              TrouserModel = 5605934,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_928]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_3061]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_3064]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_3056]]),  
                        Action = {
                          InstanceId = [[Client1_3063]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_3062]],  
                      Class = [[EventType]],  
                      Type = [[targeted by player]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_946]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_947]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Wander]],  
                        ActivityZoneId = r2.RefId([[Client1_933]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_4152]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_4161]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_4154]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_931]],  
                Class = [[Position]],  
                x = 29776.46875,  
                y = -1408.265625,  
                z = 75
              }
            },  
            {
              InstanceId = [[Client1_977]],  
              Class = [[NpcCustom]],  
              Angle = 1.25,  
              ArmColor = 4,  
              ArmModel = 0,  
              Base = [[palette.entities.npcs.civils.f_civil_70]],  
              EyesColor = 7,  
              FeetColor = 0,  
              FeetModel = 6702382,  
              GabaritArmsWidth = 6,  
              GabaritBreastSize = 14,  
              GabaritHeight = 0,  
              GabaritLegsWidth = 4,  
              GabaritTorsoWidth = 3,  
              HairColor = 3,  
              HairType = 5621550,  
              HandsColor = 3,  
              HandsModel = 5605678,  
              InheritPos = 1,  
              JacketColor = 0,  
              JacketModel = 5606446,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 4,  
              MorphTarget2 = 5,  
              MorphTarget3 = 0,  
              MorphTarget4 = 1,  
              MorphTarget5 = 6,  
              MorphTarget6 = 6,  
              MorphTarget7 = 3,  
              MorphTarget8 = 5,  
              Name = [[Eunix]],  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_civil_light_melee_blunt_c2.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 0,  
              Tattoo = 25,  
              TrouserColor = 2,  
              TrouserModel = 5605934,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_975]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_4900]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_4906]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Stand Still]],  
                        ActivityZoneId = r2.RefId([[]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_4907]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_4908]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_4887]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_978]],  
                Class = [[Position]],  
                x = 29780.95313,  
                y = -1454.359375,  
                z = 74.09375
              }
            },  
            {
              InstanceId = [[Client1_193]],  
              Class = [[NpcCustom]],  
              Angle = 2.109375,  
              ArmColor = 5,  
              ArmModel = 6703918,  
              Base = [[palette.entities.npcs.civils.f_civil_70]],  
              EyesColor = 1,  
              FeetColor = 5,  
              FeetModel = 6702382,  
              GabaritArmsWidth = 8,  
              GabaritBreastSize = 10,  
              GabaritHeight = 9,  
              GabaritLegsWidth = 7,  
              GabaritTorsoWidth = 4,  
              HairColor = 2,  
              HairType = 3118,  
              HandsColor = 5,  
              HandsModel = 6702894,  
              InheritPos = 1,  
              JacketColor = 0,  
              JacketModel = 6704430,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 4,  
              MorphTarget2 = 3,  
              MorphTarget3 = 3,  
              MorphTarget4 = 1,  
              MorphTarget5 = 2,  
              MorphTarget6 = 3,  
              MorphTarget7 = 4,  
              MorphTarget8 = 3,  
              Name = [[Bastosh]],  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_civil_light_melee_blunt_c2.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 0,  
              Tattoo = 20,  
              TrouserColor = 4,  
              TrouserModel = 6703406,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_191]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_21743]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_21750]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_21735]]),  
                        Action = {
                          InstanceId = [[Client1_21749]],  
                          Class = [[ActionType]],  
                          Type = [[deactivate]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_21744]],  
                      Class = [[EventType]],  
                      Type = [[targeted by player]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                },  
                ChatSequences = {
                }
              },  
              Position = {
                InstanceId = [[Client1_194]],  
                Class = [[Position]],  
                x = 29769.23438,  
                y = -1462.0625,  
                z = 73.640625
              }
            },  
            {
              InstanceId = [[Client1_426]],  
              Class = [[NpcCreature]],  
              Angle = -2.209217548,  
              Base = [[palette.entities.creatures.chbdc1]],  
              BotAttackable = 0,  
              InheritPos = 1,  
              Name = [[Bobo]],  
              PlayerAttackable = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_424]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_452]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_453]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Rest In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_430]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[10]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      },  
                      {
                        InstanceId = [[Client1_454]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Feed In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_430]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[30]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_428]],  
                Class = [[Position]],  
                x = 29782.64063,  
                y = -1380.0625,  
                z = 74.34375
              }
            },  
            {
              InstanceId = [[Client1_5133]],  
              Class = [[NpcCreature]],  
              Angle = 0.6806784272,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.chadc1]],  
              InheritPos = 1,  
              Name = [[Boedix]],  
              PlayerAttackable = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_5131]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_5490]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_5493]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_910]]),  
                        Action = {
                          InstanceId = [[Client1_5492]],  
                          Class = [[ActionType]],  
                          Type = [[Deactivate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_5495]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_5174]]),  
                        Action = {
                          InstanceId = [[Client1_5494]],  
                          Class = [[ActionType]],  
                          Type = [[Activate]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_5491]],  
                      Class = [[EventType]],  
                      Type = [[activation]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_5135]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_5136]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Feed In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_4391]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_5134]],  
                Class = [[Position]],  
                x = 29771.42188,  
                y = -1459.984375,  
                z = 73.703125
              }
            },  
            {
              InstanceId = [[Client1_5139]],  
              Class = [[NpcCreature]],  
              Angle = 5.969026089,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.chadc1]],  
              InheritPos = 1,  
              Name = [[Eunix]],  
              PlayerAttackable = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_5140]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_5141]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_5142]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Feed In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_4391]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_5143]],  
                Class = [[Position]],  
                x = 29771.0625,  
                y = -1456.796875,  
                z = 73.71875
              }
            },  
            {
              InstanceId = [[Client1_5146]],  
              Class = [[NpcCreature]],  
              Angle = 3.263765812,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.chadc1]],  
              InheritPos = 1,  
              Name = [[Tu'Rlin]],  
              PlayerAttackable = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_5147]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_5326]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_5329]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_892]]),  
                        Action = {
                          InstanceId = [[Client1_5328]],  
                          Class = [[ActionType]],  
                          Type = [[Deactivate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_5331]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_5195]]),  
                        Action = {
                          InstanceId = [[Client1_5330]],  
                          Class = [[ActionType]],  
                          Type = [[Activate]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_5327]],  
                      Class = [[EventType]],  
                      Type = [[activation]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_5148]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_5149]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Feed In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_4391]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_5150]],  
                Class = [[Position]],  
                x = 29769.90625,  
                y = -1459.21875,  
                z = 73.640625
              }
            },  
            {
              InstanceId = [[Client1_5153]],  
              Class = [[NpcCreature]],  
              Angle = 1.312346458,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.chadc1]],  
              InheritPos = 1,  
              Name = [[Zetonia]],  
              PlayerAttackable = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_5154]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_5319]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_5322]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_313]]),  
                        Action = {
                          InstanceId = [[Client1_5321]],  
                          Class = [[ActionType]],  
                          Type = [[Deactivate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_5324]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_5146]]),  
                        Action = {
                          InstanceId = [[Client1_5323]],  
                          Class = [[ActionType]],  
                          Type = [[Activate]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_5320]],  
                      Class = [[EventType]],  
                      Type = [[activation]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_5155]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_5156]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Feed In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_4391]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_5157]],  
                Class = [[Position]],  
                x = 29769.3125,  
                y = -1456.40625,  
                z = 73.703125
              }
            },  
            {
              InstanceId = [[Client1_5160]],  
              Class = [[NpcCreature]],  
              Angle = 5.585053444,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.chadc1]],  
              InheritPos = 1,  
              Name = [[Dylion]],  
              PlayerAttackable = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_5161]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_5504]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_5507]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_977]]),  
                        Action = {
                          InstanceId = [[Client1_5506]],  
                          Class = [[ActionType]],  
                          Type = [[Deactivate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_5509]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_5139]]),  
                        Action = {
                          InstanceId = [[Client1_5508]],  
                          Class = [[ActionType]],  
                          Type = [[Activate]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_5505]],  
                      Class = [[EventType]],  
                      Type = [[activation]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_5162]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_5163]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Feed In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_4391]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_5164]],  
                Class = [[Position]],  
                x = 29768.15625,  
                y = -1458.296875,  
                z = 73.640625
              }
            },  
            {
              InstanceId = [[Client1_5167]],  
              Class = [[NpcCreature]],  
              Angle = 0,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.chadc1]],  
              InheritPos = 1,  
              Name = [[Lyrius]],  
              PlayerAttackable = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_5168]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_5483]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_5486]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_973]]),  
                        Action = {
                          InstanceId = [[Client1_5485]],  
                          Class = [[ActionType]],  
                          Type = [[Deactivate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_5488]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_5133]]),  
                        Action = {
                          InstanceId = [[Client1_5487]],  
                          Class = [[ActionType]],  
                          Type = [[Activate]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_5484]],  
                      Class = [[EventType]],  
                      Type = [[activation]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_5169]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_5170]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Feed In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_4391]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_5171]],  
                Class = [[Position]],  
                x = 29767.29688,  
                y = -1456.59375,  
                z = 73.71875
              }
            },  
            {
              InstanceId = [[Client1_5174]],  
              Class = [[NpcCreature]],  
              Angle = 3.001966238,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.chadc1]],  
              InheritPos = 1,  
              Name = [[Diorus]],  
              PlayerAttackable = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_5175]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_5497]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_5500]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_930]]),  
                        Action = {
                          InstanceId = [[Client1_5499]],  
                          Class = [[ActionType]],  
                          Type = [[Deactivate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_5502]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_5160]]),  
                        Action = {
                          InstanceId = [[Client1_5501]],  
                          Class = [[ActionType]],  
                          Type = [[Activate]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_5498]],  
                      Class = [[EventType]],  
                      Type = [[activation]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_5176]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_5177]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Feed In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_4391]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_5178]],  
                Class = [[Position]],  
                x = 29766.46875,  
                y = -1458.65625,  
                z = 73.59375
              }
            },  
            {
              InstanceId = [[Client1_5181]],  
              Class = [[NpcCreature]],  
              Angle = 1.884955645,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.chadc1]],  
              InheritPos = 1,  
              Name = [[Xathus]],  
              PlayerAttackable = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_5182]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_5469]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_5472]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_950]]),  
                        Action = {
                          InstanceId = [[Client1_5471]],  
                          Class = [[ActionType]],  
                          Type = [[Deactivate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_5474]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_5202]]),  
                        Action = {
                          InstanceId = [[Client1_5473]],  
                          Class = [[ActionType]],  
                          Type = [[Activate]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_5470]],  
                      Class = [[EventType]],  
                      Type = [[activation]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_5183]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_5184]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Feed In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_4391]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_5185]],  
                Class = [[Position]],  
                x = 29765.14063,  
                y = -1456.859375,  
                z = 73.734375
              }
            },  
            {
              InstanceId = [[Client1_5188]],  
              Class = [[NpcCreature]],  
              Angle = 1.312346458,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.chadc1]],  
              InheritPos = 1,  
              Name = [[Pirus]],  
              PlayerAttackable = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_5189]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_5340]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_5343]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_904]]),  
                        Action = {
                          InstanceId = [[Client1_5342]],  
                          Class = [[ActionType]],  
                          Type = [[Deactivate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_5345]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_5181]]),  
                        Action = {
                          InstanceId = [[Client1_5344]],  
                          Class = [[ActionType]],  
                          Type = [[Activate]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_5341]],  
                      Class = [[EventType]],  
                      Type = [[activation]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_5190]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_5191]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Feed In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_4391]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_5192]],  
                Class = [[Position]],  
                x = 29766.8125,  
                y = -1460.71875,  
                z = 73.5625
              }
            },  
            {
              InstanceId = [[Client1_5195]],  
              Class = [[NpcCreature]],  
              Angle = 3.316125631,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.chadc1]],  
              InheritPos = 1,  
              Name = [[Ulydix]],  
              PlayerAttackable = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_5196]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_5333]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_5336]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_898]]),  
                        Action = {
                          InstanceId = [[Client1_5335]],  
                          Class = [[ActionType]],  
                          Type = [[Deactivate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_5338]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_5188]]),  
                        Action = {
                          InstanceId = [[Client1_5337]],  
                          Class = [[ActionType]],  
                          Type = [[Activate]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_5334]],  
                      Class = [[EventType]],  
                      Type = [[activation]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_5197]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_5198]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Feed In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_4391]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_5199]],  
                Class = [[Position]],  
                x = 29764.48438,  
                y = -1459.28125,  
                z = 73.578125
              }
            },  
            {
              InstanceId = [[Client1_5202]],  
              Class = [[NpcCreature]],  
              Angle = 0.5585053563,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.chadc1]],  
              InheritPos = 1,  
              Name = [[Guedon]],  
              PlayerAttackable = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_5203]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_5476]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_5479]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_981]]),  
                        Action = {
                          InstanceId = [[Client1_5478]],  
                          Class = [[ActionType]],  
                          Type = [[Deactivate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_5481]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_5167]]),  
                        Action = {
                          InstanceId = [[Client1_5480]],  
                          Class = [[ActionType]],  
                          Type = [[Activate]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_5477]],  
                      Class = [[EventType]],  
                      Type = [[activation]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_5204]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_5205]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Feed In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_4391]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_5206]],  
                Class = [[Position]],  
                x = 29764.95313,  
                y = -1461.28125,  
                z = 73.53125
              }
            },  
            {
              InstanceId = [[Client1_5532]],  
              Class = [[NpcCustom]],  
              Aggro = 50,  
              Angle = 2.015625,  
              ArmColor = 0,  
              ArmModel = 0,  
              AutoSpawn = 0,  
              Base = [[palette.entities.npcs.bandits.f_light_melee_70]],  
              BotAttackable = 1,  
              EyesColor = 1,  
              FeetColor = 5,  
              FeetModel = 6702382,  
              GabaritArmsWidth = 8,  
              GabaritBreastSize = 10,  
              GabaritHeight = 9,  
              GabaritLegsWidth = 7,  
              GabaritTorsoWidth = 4,  
              HairColor = 2,  
              HairType = 3118,  
              HandsColor = 5,  
              HandsModel = 6702894,  
              InheritPos = 1,  
              JacketColor = 0,  
              JacketModel = 6704430,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 4,  
              MorphTarget2 = 3,  
              MorphTarget3 = 3,  
              MorphTarget4 = 1,  
              MorphTarget5 = 2,  
              MorphTarget6 = 3,  
              MorphTarget7 = 4,  
              MorphTarget8 = 3,  
              Name = [[Bastosh]],  
              NoRespawn = 1,  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_light_melee_pierce_c4.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 1,  
              Tattoo = 20,  
              TrouserColor = 4,  
              TrouserModel = 6703406,  
              WeaponLeftHand = 6753070,  
              WeaponRightHand = 6753070,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_5530]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_5751]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_5754]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_2776]]),  
                        Action = {
                          InstanceId = [[Client1_5753]],  
                          Class = [[ActionType]],  
                          Type = [[deactivate]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_5752]],  
                      Class = [[EventType]],  
                      Type = [[death]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_6249]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_6252]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_4183]]),  
                        Action = {
                          InstanceId = [[Client1_6251]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_22760]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_22755]]),  
                        Action = {
                          InstanceId = [[Client1_22759]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_23003]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_5757]]),  
                        Action = {
                          InstanceId = [[Client1_23002]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_6250]],  
                      Class = [[EventType]],  
                      Type = [[death]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                },  
                ChatSequences = {
                }
              },  
              Position = {
                InstanceId = [[Client1_5533]],  
                Class = [[Position]],  
                x = 29769.26563,  
                y = -1462.109375,  
                z = 73.640625
              }
            },  
            {
              InstanceId = [[Client1_313]],  
              Class = [[NpcCustom]],  
              Angle = -2.90625,  
              ArmColor = 0,  
              ArmModel = 0,  
              Base = [[palette.entities.npcs.civils.f_civil_70]],  
              EyesColor = 3,  
              FeetColor = 4,  
              FeetModel = 6702382,  
              GabaritArmsWidth = 0,  
              GabaritBreastSize = 7,  
              GabaritHeight = 8,  
              GabaritLegsWidth = 0,  
              GabaritTorsoWidth = 0,  
              HairColor = 0,  
              HairType = 5621550,  
              HandsColor = 0,  
              HandsModel = 0,  
              InheritPos = 1,  
              JacketColor = 4,  
              JacketModel = 6704430,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 5,  
              MorphTarget2 = 3,  
              MorphTarget3 = 0,  
              MorphTarget4 = 0,  
              MorphTarget5 = 5,  
              MorphTarget6 = 5,  
              MorphTarget7 = 0,  
              MorphTarget8 = 4,  
              Name = [[Tu'Rlin]],  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_civil_light_melee_blunt_c2.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 0,  
              Tattoo = 23,  
              TrouserColor = 2,  
              TrouserModel = 6703406,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_311]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_1621]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_1624]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_1599]]),  
                        Action = {
                          InstanceId = [[Client1_1623]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_1622]],  
                      Class = [[EventType]],  
                      Type = [[targeted by player]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_4137]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_4138]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Stand Still]],  
                        ActivityZoneId = r2.RefId([[]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_4139]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_4151]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_4141]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_314]],  
                Class = [[Position]],  
                x = 29782.71875,  
                y = -1433.359375,  
                z = 73.65625
              }
            },  
            {
              InstanceId = [[Client1_457]],  
              Class = [[NpcCustom]],  
              Angle = -2.6875,  
              ArmColor = 0,  
              ArmModel = 5606190,  
              Base = [[palette.entities.npcs.civils.f_civil_70]],  
              EyesColor = 4,  
              FeetColor = 1,  
              FeetModel = 5605422,  
              GabaritArmsWidth = 5,  
              GabaritBreastSize = 13,  
              GabaritHeight = 14,  
              GabaritLegsWidth = 4,  
              GabaritTorsoWidth = 6,  
              HairColor = 5,  
              HairType = 5621806,  
              HandsColor = 1,  
              HandsModel = 5605678,  
              InheritPos = 1,  
              JacketColor = 0,  
              JacketModel = 5606446,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 5,  
              MorphTarget2 = 4,  
              MorphTarget3 = 4,  
              MorphTarget4 = 2,  
              MorphTarget5 = 2,  
              MorphTarget6 = 5,  
              MorphTarget7 = 4,  
              MorphTarget8 = 5,  
              Name = [[Zetonia]],  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_civil_light_melee_blunt_c2.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 0,  
              Tattoo = 19,  
              TrouserColor = 1,  
              TrouserModel = 5605934,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_455]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_1353]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_1356]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_1348]]),  
                        Action = {
                          InstanceId = [[Client1_1355]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_1354]],  
                      Class = [[EventType]],  
                      Type = [[targeted by player]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_4117]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_4118]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Stand Still]],  
                        ActivityZoneId = r2.RefId([[]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_4119]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_4128]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_4121]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_458]],  
                Class = [[Position]],  
                x = 29778.375,  
                y = -1382.171875,  
                z = 74.4375
              }
            },  
            {
              InstanceId = [[Client1_513]],  
              Class = [[Npc]],  
              Angle = -0.890625,  
              Base = [[palette.entities.botobjects.wind_turbine]],  
              InheritPos = 1,  
              Name = [[wind turbine 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_511]],  
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
                InstanceId = [[Client1_514]],  
                Class = [[Position]],  
                x = 29711.40625,  
                y = -1405.1875,  
                z = 78.375
              }
            },  
            {
              InstanceId = [[Client1_683]],  
              Class = [[NpcCustom]],  
              Angle = -0.8125,  
              ArmColor = 3,  
              ArmModel = 6703918,  
              Base = [[palette.entities.npcs.civils.f_civil_70]],  
              EyesColor = 7,  
              FeetColor = 4,  
              FeetModel = 6702382,  
              GabaritArmsWidth = 9,  
              GabaritBreastSize = 0,  
              GabaritHeight = 11,  
              GabaritLegsWidth = 8,  
              GabaritTorsoWidth = 3,  
              HairColor = 1,  
              HairType = 5623598,  
              HandsColor = 3,  
              HandsModel = 6702894,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6704430,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 5,  
              MorphTarget2 = 1,  
              MorphTarget3 = 4,  
              MorphTarget4 = 6,  
              MorphTarget5 = 4,  
              MorphTarget6 = 4,  
              MorphTarget7 = 3,  
              MorphTarget8 = 5,  
              Name = [[O'Darghan]],  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_civil_light_melee_blunt_c2.creature]],  
              SheetClient = [[basic_tryker_male.creature]],  
              Speed = 0,  
              Tattoo = 23,  
              TrouserColor = 4,  
              TrouserModel = 6703406,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_681]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_5790]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_5793]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_2635]]),  
                        Action = {
                          InstanceId = [[Client1_5792]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_5791]],  
                      Class = [[EventType]],  
                      Type = [[targeted by player]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_22842]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_22845]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_5757]]),  
                        Action = {
                          InstanceId = [[Client1_22844]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_22843]],  
                      Class = [[EventType]],  
                      Type = [[targeted by player]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                },  
                ChatSequences = {
                }
              },  
              Position = {
                InstanceId = [[Client1_684]],  
                Class = [[Position]],  
                x = 29657.60938,  
                y = -1110.421875,  
                z = 74.59375
              }
            },  
            {
              InstanceId = [[Client1_709]],  
              Class = [[NpcPlant]],  
              Angle = -0.3493981659,  
              Base = [[palette.entities.creatures.cpfdc2]],  
              InheritPos = 1,  
              Name = [[Blooming Shooki]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_707]],  
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
                InstanceId = [[Client1_710]],  
                Class = [[Position]],  
                x = 29626.20313,  
                y = -1109.5625,  
                z = 75.0625
              }
            },  
            {
              InstanceId = [[Client1_737]],  
              Class = [[NpcPlant]],  
              Angle = -1.657830477,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.cpfdc4]],  
              InheritPos = 1,  
              Name = [[Famished Shooki]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_735]],  
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
                InstanceId = [[Client1_738]],  
                Class = [[Position]],  
                x = 29630.75,  
                y = -1086.859375,  
                z = 77.390625
              }
            },  
            {
              InstanceId = [[Client1_741]],  
              Class = [[NpcPlant]],  
              Angle = -1.766483784,  
              Base = [[palette.entities.creatures.cpfdc3]],  
              InheritPos = 1,  
              Name = [[Dehydrated Shooki]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_739]],  
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
                InstanceId = [[Client1_742]],  
                Class = [[Position]],  
                x = 29642.34375,  
                y = -1064.1875,  
                z = 76.03125
              }
            },  
            {
              InstanceId = [[Client1_745]],  
              Class = [[NpcPlant]],  
              Angle = -1.150203466,  
              Base = [[palette.entities.creatures.cpfdc2]],  
              InheritPos = 1,  
              Name = [[Blooming Shooki]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_743]],  
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
                InstanceId = [[Client1_746]],  
                Class = [[Position]],  
                x = 29654.89063,  
                y = -1087.703125,  
                z = 73.796875
              }
            },  
            {
              InstanceId = [[Client1_749]],  
              Class = [[NpcPlant]],  
              Angle = -0.3680036366,  
              Base = [[palette.entities.creatures.cpfdc3]],  
              InheritPos = 1,  
              Name = [[Dehydrated Shooki]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_747]],  
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
                InstanceId = [[Client1_750]],  
                Class = [[Position]],  
                x = 29605.25,  
                y = -1076.375,  
                z = 75.265625
              }
            },  
            {
              InstanceId = [[Client1_892]],  
              Class = [[NpcCustom]],  
              Angle = 0.96875,  
              ArmColor = 2,  
              ArmModel = 0,  
              Base = [[palette.entities.npcs.civils.f_civil_70]],  
              EyesColor = 6,  
              FeetColor = 1,  
              FeetModel = 6702382,  
              GabaritArmsWidth = 12,  
              GabaritBreastSize = 6,  
              GabaritHeight = 6,  
              GabaritLegsWidth = 6,  
              GabaritTorsoWidth = 5,  
              HairColor = 2,  
              HairType = 5621550,  
              HandsColor = 4,  
              HandsModel = 0,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6704430,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 6,  
              MorphTarget2 = 5,  
              MorphTarget3 = 6,  
              MorphTarget4 = 2,  
              MorphTarget5 = 5,  
              MorphTarget6 = 5,  
              MorphTarget7 = 5,  
              MorphTarget8 = 1,  
              Name = [[Ulydix]],  
              Sex = 1,  
              Sheet = [[ring_civil_light_melee_blunt_c2.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 0,  
              Tattoo = 0,  
              TrouserColor = 0,  
              TrouserModel = 6703406,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_890]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_3043]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_3046]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_3039]]),  
                        Action = {
                          InstanceId = [[Client1_3045]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_3044]],  
                      Class = [[EventType]],  
                      Type = [[targeted by player]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_894]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_895]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Wander]],  
                        ActivityZoneId = r2.RefId([[Client1_825]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_4842]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_4843]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_4834]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_893]],  
                Class = [[Position]],  
                x = 29728.90625,  
                y = -1451.359375,  
                z = 75
              }
            },  
            {
              InstanceId = [[Client1_898]],  
              Class = [[NpcCustom]],  
              Angle = -0.78125,  
              ArmColor = 3,  
              ArmModel = 5606190,  
              Base = [[palette.entities.npcs.civils.f_civil_70]],  
              EyesColor = 4,  
              FeetColor = 0,  
              FeetModel = 5605422,  
              GabaritArmsWidth = 2,  
              GabaritBreastSize = 2,  
              GabaritHeight = 5,  
              GabaritLegsWidth = 6,  
              GabaritTorsoWidth = 3,  
              HairColor = 5,  
              HairType = 5622062,  
              HandsColor = 3,  
              HandsModel = 0,  
              InheritPos = 1,  
              JacketColor = 1,  
              JacketModel = 6704430,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 4,  
              MorphTarget2 = 7,  
              MorphTarget3 = 3,  
              MorphTarget4 = 4,  
              MorphTarget5 = 1,  
              MorphTarget6 = 2,  
              MorphTarget7 = 1,  
              MorphTarget8 = 1,  
              Name = [[Pirus]],  
              Sex = 0,  
              Sheet = [[ring_civil_light_melee_blunt_c2.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 0,  
              Tattoo = 9,  
              TrouserColor = 4,  
              TrouserModel = 5605934,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_896]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_3033]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_3036]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_3028]]),  
                        Action = {
                          InstanceId = [[Client1_3035]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_3034]],  
                      Class = [[EventType]],  
                      Type = [[targeted by player]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_900]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_901]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Wander]],  
                        ActivityZoneId = r2.RefId([[Client1_851]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_4823]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_4832]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_4825]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_899]],  
                Class = [[Position]],  
                x = 29716.76563,  
                y = -1422.53125,  
                z = 76.171875
              }
            },  
            {
              InstanceId = [[Client1_904]],  
              Class = [[NpcCustom]],  
              Angle = -0.5625,  
              ArmColor = 0,  
              ArmModel = 0,  
              Base = [[palette.entities.npcs.civils.f_civil_70]],  
              EyesColor = 7,  
              FeetColor = 0,  
              FeetModel = 5605422,  
              GabaritArmsWidth = 1,  
              GabaritBreastSize = 4,  
              GabaritHeight = 4,  
              GabaritLegsWidth = 12,  
              GabaritTorsoWidth = 7,  
              HairColor = 1,  
              HairType = 5621806,  
              HandsColor = 0,  
              HandsModel = 5605678,  
              InheritPos = 1,  
              JacketColor = 5,  
              JacketModel = 5606446,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 3,  
              MorphTarget2 = 3,  
              MorphTarget3 = 4,  
              MorphTarget4 = 6,  
              MorphTarget5 = 1,  
              MorphTarget6 = 6,  
              MorphTarget7 = 5,  
              MorphTarget8 = 4,  
              Name = [[Xathus]],  
              Sex = 1,  
              Sheet = [[ring_civil_light_melee_blunt_c2.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 0,  
              Tattoo = 12,  
              TrouserColor = 5,  
              TrouserModel = 5605934,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_902]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_3022]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_3025]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_3017]]),  
                        Action = {
                          InstanceId = [[Client1_3024]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_3023]],  
                      Class = [[EventType]],  
                      Type = [[targeted by player]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_906]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_907]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Wander]],  
                        ActivityZoneId = r2.RefId([[Client1_871]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_4804]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_4813]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_4806]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_905]],  
                Class = [[Position]],  
                x = 29729.40625,  
                y = -1408.1875,  
                z = 80.140625
              }
            },  
            {
              InstanceId = [[Client1_950]],  
              Class = [[NpcCustom]],  
              Angle = 1.484375,  
              ArmColor = 5,  
              ArmModel = 0,  
              Base = [[palette.entities.npcs.civils.f_civil_70]],  
              EyesColor = 7,  
              FeetColor = 4,  
              FeetModel = 6702382,  
              GabaritArmsWidth = 7,  
              GabaritBreastSize = 3,  
              GabaritHeight = 11,  
              GabaritLegsWidth = 8,  
              GabaritTorsoWidth = 1,  
              HairColor = 2,  
              HairType = 3118,  
              HandsColor = 0,  
              HandsModel = 0,  
              InheritPos = 1,  
              JacketColor = 2,  
              JacketModel = 5606446,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 3,  
              MorphTarget2 = 6,  
              MorphTarget3 = 1,  
              MorphTarget4 = 7,  
              MorphTarget5 = 2,  
              MorphTarget6 = 2,  
              MorphTarget7 = 3,  
              MorphTarget8 = 3,  
              Name = [[Guedon]],  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_civil_light_melee_blunt_c2.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 0,  
              Tattoo = 19,  
              TrouserColor = 2,  
              TrouserModel = 5605934,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_948]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_3066]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_3069]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_3050]]),  
                        Action = {
                          InstanceId = [[Client1_3068]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_3067]],  
                      Class = [[EventType]],  
                      Type = [[targeted by player]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_21787]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_21788]],  
                      Class = [[EventType]],  
                      Type = [[targeted by player]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_21795]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_21798]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_21782]]),  
                        Action = {
                          InstanceId = [[Client1_21797]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_21796]],  
                      Class = [[EventType]],  
                      Type = [[targeted by player]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_969]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_970]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Wander]],  
                        ActivityZoneId = r2.RefId([[Client1_953]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_4853]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_4854]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_4845]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_951]],  
                Class = [[Position]],  
                x = 29748.28125,  
                y = -1457.984375,  
                z = 74.140625
              }
            },  
            {
              InstanceId = [[Client1_981]],  
              Class = [[NpcCustom]],  
              Angle = 2.875,  
              ArmColor = 0,  
              ArmModel = 0,  
              Base = [[palette.entities.npcs.civils.f_civil_70]],  
              EyesColor = 3,  
              FeetColor = 2,  
              FeetModel = 5605422,  
              GabaritArmsWidth = 0,  
              GabaritBreastSize = 4,  
              GabaritHeight = 10,  
              GabaritLegsWidth = 5,  
              GabaritTorsoWidth = 3,  
              HairColor = 2,  
              HairType = 2606,  
              HandsColor = 1,  
              HandsModel = 0,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 5606446,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 7,  
              MorphTarget2 = 5,  
              MorphTarget3 = 6,  
              MorphTarget4 = 2,  
              MorphTarget5 = 4,  
              MorphTarget6 = 0,  
              MorphTarget7 = 2,  
              MorphTarget8 = 2,  
              Name = [[Lyrius]],  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_civil_light_melee_blunt_c2.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 0,  
              Tattoo = 0,  
              TrouserColor = 2,  
              TrouserModel = 6703406,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_979]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_3263]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_3266]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_3261]]),  
                        Action = {
                          InstanceId = [[Client1_3265]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_3264]],  
                      Class = [[EventType]],  
                      Type = [[targeted by player]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_21805]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_21808]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_3261]]),  
                        Action = {
                          InstanceId = [[Client1_21807]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_21806]],  
                      Class = [[EventType]],  
                      Type = [[targeted by player]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_21880]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_21883]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_21801]]),  
                        Action = {
                          InstanceId = [[Client1_21882]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_21881]],  
                      Class = [[EventType]],  
                      Type = [[targeted by player]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_1000]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_1001]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Wander]],  
                        ActivityZoneId = r2.RefId([[Client1_984]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_4855]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_4864]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_4857]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_982]],  
                Class = [[Position]],  
                x = 29765.09375,  
                y = -1429.78125,  
                z = 74.8125
              }
            },  
            {
              InstanceId = [[Client1_1217]],  
              Class = [[NpcCreature]],  
              Angle = -0.5568241477,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ccbdc7]],  
              InheritPos = 1,  
              Name = [[Clopperketh]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1215]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_1632]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_1635]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_1628]]),  
                        Action = {
                          InstanceId = [[Client1_1634]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_1637]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_1599]]),  
                        Action = {
                          InstanceId = [[Client1_1636]],  
                          Class = [[ActionType]],  
                          Type = [[deactivate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_22768]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_22763]]),  
                        Action = {
                          InstanceId = [[Client1_22767]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_1633]],  
                      Class = [[EventType]],  
                      Type = [[targeted by player]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_1322]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_1323]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Guard Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_1163]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      },  
                      {
                        InstanceId = [[Client1_1324]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Guard Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_1200]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      },  
                      {
                        InstanceId = [[Client1_1325]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Guard Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_1126]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      },  
                      {
                        InstanceId = [[Client1_1326]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Guard Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_1089]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_1218]],  
                Class = [[Position]],  
                x = 29847.14063,  
                y = -1207.109375,  
                z = 61.8125
              }
            },  
            {
              InstanceId = [[Client1_973]],  
              Class = [[NpcCustom]],  
              Angle = 2.953125,  
              ArmColor = 0,  
              ArmModel = 5606190,  
              Base = [[palette.entities.npcs.civils.f_civil_70]],  
              EyesColor = 1,  
              FeetColor = 3,  
              FeetModel = 5605422,  
              GabaritArmsWidth = 6,  
              GabaritBreastSize = 12,  
              GabaritHeight = 12,  
              GabaritLegsWidth = 8,  
              GabaritTorsoWidth = 4,  
              HairColor = 1,  
              HairType = 3118,  
              HandsColor = 1,  
              HandsModel = 6702894,  
              InheritPos = 1,  
              JacketColor = 0,  
              JacketModel = 5606446,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 7,  
              MorphTarget2 = 6,  
              MorphTarget3 = 6,  
              MorphTarget4 = 2,  
              MorphTarget5 = 1,  
              MorphTarget6 = 0,  
              MorphTarget7 = 1,  
              MorphTarget8 = 7,  
              Name = [[Boedix]],  
              Sex = 1,  
              Sheet = [[ring_civil_light_melee_blunt_c2.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 0,  
              Tattoo = 20,  
              TrouserColor = 4,  
              TrouserModel = 5605934,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_971]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_4865]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_4866]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Stand Still]],  
                        ActivityZoneId = r2.RefId([[]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_4867]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_4879]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_4869]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_974]],  
                Class = [[Position]],  
                x = 29783.10938,  
                y = -1451.8125,  
                z = 74.109375
              }
            },  
            {
              InstanceId = [[Client1_679]],  
              Class = [[NpcCustom]],  
              Angle = -0.78125,  
              ArmColor = 3,  
              ArmModel = 6703918,  
              Base = [[palette.entities.npcs.civils.f_civil_70]],  
              EyesColor = 2,  
              FeetColor = 4,  
              FeetModel = 6702382,  
              GabaritArmsWidth = 6,  
              GabaritBreastSize = 0,  
              GabaritHeight = 8,  
              GabaritLegsWidth = 7,  
              GabaritTorsoWidth = 2,  
              HairColor = 1,  
              HairType = 2606,  
              HandsColor = 4,  
              HandsModel = 6702894,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6704430,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 5,  
              MorphTarget2 = 3,  
              MorphTarget3 = 1,  
              MorphTarget4 = 3,  
              MorphTarget5 = 5,  
              MorphTarget6 = 6,  
              MorphTarget7 = 2,  
              MorphTarget8 = 4,  
              Name = [[Dekos]],  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_civil_light_melee_blunt_c2.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 0,  
              Tattoo = 12,  
              TrouserColor = 4,  
              TrouserModel = 6703406,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_677]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_2629]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_2632]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_2597]]),  
                        Action = {
                          InstanceId = [[Client1_2631]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_21980]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_2635]]),  
                        Action = {
                          InstanceId = [[Client1_21979]],  
                          Class = [[ActionType]],  
                          Type = [[deactivate]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                      {
                        InstanceId = [[Client1_23189]],  
                        Class = [[ConditionStep]],  
                        Entity = r2.RefId([[Client1_2597]]),  
                        Condition = {
                          InstanceId = [[Client1_23188]],  
                          Class = [[ConditionType]],  
                          Type = [[is not in dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Event = {
                      InstanceId = [[Client1_2630]],  
                      Class = [[EventType]],  
                      Type = [[targeted by player]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                },  
                ChatSequences = {
                }
              },  
              Position = {
                InstanceId = [[Client1_680]],  
                Class = [[Position]],  
                x = 29655.34375,  
                y = -1113.375,  
                z = 74.8125
              }
            },  
            {
              InstanceId = [[Client1_23057]],  
              Class = [[NpcCreature]],  
              Angle = 0.6905971766,  
              Base = [[palette.entities.creatures.cccdc3]],  
              InheritPos = 1,  
              Name = [[Dangerous Goari]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_23055]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_23059]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_23060]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Hunt In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_22343]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_23058]],  
                Class = [[Position]],  
                x = 29494.125,  
                y = -1468.359375,  
                z = 74.890625
              }
            },  
            {
              InstanceId = [[Client1_23063]],  
              Class = [[NpcCreature]],  
              Angle = 0.6905971766,  
              Base = [[palette.entities.creatures.cccdc3]],  
              InheritPos = 1,  
              Name = [[Dangerous Goari]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_23061]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_23065]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_23066]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Hunt In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_22343]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_23064]],  
                Class = [[Position]],  
                x = 29534.07813,  
                y = -1495.65625,  
                z = 76.0625
              }
            },  
            {
              InstanceId = [[Client1_23069]],  
              Class = [[NpcCreature]],  
              Angle = 0.6905971766,  
              Base = [[palette.entities.creatures.cccdc3]],  
              InheritPos = 1,  
              Name = [[Dangerous Goari]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_23067]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_23071]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_23072]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Hunt In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_22343]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_23070]],  
                Class = [[Position]],  
                x = 29557.48438,  
                y = -1538.546875,  
                z = 75.875
              }
            },  
            {
              InstanceId = [[Client1_23075]],  
              Class = [[NpcCreature]],  
              Angle = 0.6905971766,  
              Base = [[palette.entities.creatures.cccdc2]],  
              InheritPos = 1,  
              Name = [[Malicious Goari]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_23073]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_23077]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_23078]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Hunt In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_22343]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_23076]],  
                Class = [[Position]],  
                x = 29500.9375,  
                y = -1428.40625,  
                z = 75.015625
              }
            },  
            {
              InstanceId = [[Client1_23081]],  
              Class = [[NpcCreature]],  
              Angle = 0.6905971766,  
              Base = [[palette.entities.creatures.cccdc2]],  
              InheritPos = 1,  
              Name = [[Malicious Goari]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_23079]],  
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
                InstanceId = [[Client1_23082]],  
                Class = [[Position]],  
                x = 29546.75,  
                y = -1463.484375,  
                z = 75.8125
              }
            },  
            {
              InstanceId = [[Client1_23085]],  
              Class = [[NpcCreature]],  
              Angle = 0.6905971766,  
              Base = [[palette.entities.creatures.cccdc2]],  
              InheritPos = 1,  
              Name = [[Malicious Goari]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_23083]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_23087]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_23088]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Hunt In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_22343]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_23086]],  
                Class = [[Position]],  
                x = 29588.67188,  
                y = -1528.796875,  
                z = 74.921875
              }
            },  
            {
              InstanceId = [[Client1_23091]],  
              Class = [[NpcCreature]],  
              Angle = 0.6905971766,  
              Base = [[palette.entities.creatures.cccdc1]],  
              InheritPos = 1,  
              Name = [[Scary Goari]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_23089]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_23093]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_23094]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Hunt In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_22343]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_23092]],  
                Class = [[Position]],  
                x = 29520.4375,  
                y = -1419.625,  
                z = 74.890625
              }
            },  
            {
              InstanceId = [[Client1_23097]],  
              Class = [[NpcCreature]],  
              Angle = 0.6905971766,  
              Base = [[palette.entities.creatures.cccdc1]],  
              InheritPos = 1,  
              Name = [[Scary Goari]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_23095]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_23099]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_23100]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Hunt In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_22343]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_23098]],  
                Class = [[Position]],  
                x = 29561.375,  
                y = -1421.578125,  
                z = 77.171875
              }
            },  
            {
              InstanceId = [[Client1_23103]],  
              Class = [[NpcCreature]],  
              Angle = 0.6905971766,  
              Base = [[palette.entities.creatures.cccdc1]],  
              InheritPos = 1,  
              Name = [[Scary Goari]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_23101]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_23105]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_23106]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Hunt In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_22343]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_23104]],  
                Class = [[Position]],  
                x = 29594.51563,  
                y = -1453.75,  
                z = 75.421875
              }
            },  
            {
              InstanceId = [[Client1_23109]],  
              Class = [[NpcCreature]],  
              Angle = 0.6905971766,  
              Base = [[palette.entities.creatures.cccdc1]],  
              InheritPos = 1,  
              Name = [[Scary Goari]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_23107]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_23111]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_23112]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Hunt In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_22343]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_23110]],  
                Class = [[Position]],  
                x = 29623.76563,  
                y = -1514.1875,  
                z = 75.28125
              }
            }
          }
        },  
        {
          InstanceId = [[Client1_2635]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Dialog O'Darghan]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_2633]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_2645]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_2648]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2597]]),  
                    Action = {
                      InstanceId = [[Client1_2647]],  
                      Class = [[ActionType]],  
                      Type = [[starts dialog]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_2646]],  
                  Class = [[EventType]],  
                  Type = [[end of dialog]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_2636]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_2637]],  
                  Class = [[ChatAction]],  
                  Emote = [[Bow]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_2638]],  
                  Who = r2.RefId([[Client1_683]]),  
                  WhoNoEntity = [[]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_2634]],  
            Class = [[Position]],  
            x = 29656.57813,  
            y = -1110.765625,  
            z = 74.65625
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_2776]],  
          Class = [[ZoneTrigger]],  
          Active = 0,  
          Base = [[palette.entities.botobjects.trigger_zone]],  
          Cyclic = 0,  
          InheritPos = 1,  
          Name = [[Zone Trigger Sap]],  
          _Zone = [[Client1_2780]],  
          Behavior = {
            InstanceId = [[Client1_2777]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_2930]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_2933]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2795]]),  
                    Action = {
                      InstanceId = [[Client1_2932]],  
                      Class = [[ActionType]],  
                      Type = [[starts dialog]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_2935]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2597]]),  
                    Action = {
                      InstanceId = [[Client1_2934]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_2937]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2635]]),  
                    Action = {
                      InstanceId = [[Client1_2936]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_2931]],  
                  Class = [[EventType]],  
                  Type = [[On Player Arrived]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_2780]],  
              Class = [[Region]],  
              Deletable = 0,  
              InheritPos = 1,  
              Name = [[Places 2]],  
              Points = {
                {
                  InstanceId = [[Client1_2782]],  
                  Class = [[RegionVertex]],  
                  Deletable = 0,  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_2783]],  
                    Class = [[Position]],  
                    x = 7.921875,  
                    y = -0.90625,  
                    z = 0.1875
                  }
                },  
                {
                  InstanceId = [[Client1_2785]],  
                  Class = [[RegionVertex]],  
                  Deletable = 0,  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_2786]],  
                    Class = [[Position]],  
                    x = -0.296875,  
                    y = 11.65625,  
                    z = -0.09375
                  }
                },  
                {
                  InstanceId = [[Client1_2788]],  
                  Class = [[RegionVertex]],  
                  Deletable = 0,  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_2789]],  
                    Class = [[Position]],  
                    x = -9.421875,  
                    y = 1.3125,  
                    z = 0.03125
                  }
                },  
                {
                  InstanceId = [[Client1_2791]],  
                  Class = [[RegionVertex]],  
                  Deletable = 0,  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_2792]],  
                    Class = [[Position]],  
                    x = -0.46875,  
                    y = -8.71875,  
                    z = 0
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_2779]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_2778]],  
            Class = [[Position]],  
            x = 29656.73438,  
            y = -1113.125,  
            z = 75
          }
        },  
        {
          InstanceId = [[Client1_2795]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Dialog Sap End]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_2793]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_2920]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_2921]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_3516]],  
                  Who = r2.RefId([[Client1_683]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_2922]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_2923]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_2927]],  
                  Who = r2.RefId([[Client1_679]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_2925]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 5,  
              Actions = {
                {
                  InstanceId = [[Client1_2926]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_2928]],  
                  Who = r2.RefId([[Client1_683]]),  
                  WhoNoEntity = [[]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_2794]],  
            Class = [[Position]],  
            x = 29658.3125,  
            y = -1108.75,  
            z = 74.46875
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_3006]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Dialog Diorus]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_3004]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_3007]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_3008]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_3012]],  
                  Who = r2.RefId([[Client1_910]]),  
                  WhoNoEntity = [[]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_3005]],  
            Class = [[Position]],  
            x = 29752.9375,  
            y = -1399.359375,  
            z = 75
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_3017]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Dialog Xathus]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_3015]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_3018]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_3019]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_3020]],  
                  Who = r2.RefId([[Client1_904]]),  
                  WhoNoEntity = [[]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_3016]],  
            Class = [[Position]],  
            x = 29729.17188,  
            y = -1406.546875,  
            z = 81
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_3028]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Dialog Pirus]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_3026]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_3029]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_3030]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_23005]],  
                  Who = r2.RefId([[Client1_898]]),  
                  WhoNoEntity = [[]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_3027]],  
            Class = [[Position]],  
            x = 29716.8125,  
            y = -1420.703125,  
            z = 77
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_3039]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Dialog Ulydix]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_3037]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_3040]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_3041]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_3047]],  
                  Who = r2.RefId([[Client1_892]]),  
                  WhoNoEntity = [[]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_3038]],  
            Class = [[Position]],  
            x = 29727.42188,  
            y = -1451.484375,  
            z = 75
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_3050]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Dialog Guedon]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_3048]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_3051]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_3052]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_3053]],  
                  Who = r2.RefId([[Client1_950]]),  
                  WhoNoEntity = [[]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_3049]],  
            Class = [[Position]],  
            x = 29746.6875,  
            y = -1458.453125,  
            z = 75
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_3056]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Dialog Dylion]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_3054]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_3057]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_3058]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_3059]],  
                  Who = r2.RefId([[Client1_930]]),  
                  WhoNoEntity = [[]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_3055]],  
            Class = [[Position]],  
            x = 29779.15625,  
            y = -1409.296875,  
            z = 75
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_3261]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Dialog Lyrius]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_3259]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_3267]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_3268]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_3332]],  
                  Who = r2.RefId([[Client1_981]]),  
                  WhoNoEntity = [[]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_3260]],  
            Class = [[Position]],  
            x = 29766.0625,  
            y = -1430.359375,  
            z = 75
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_3591]],  
          Class = [[RequestItem]],  
          Active = 0,  
          Base = [[palette.entities.botobjects.bot_request_item]],  
          ContextualText = [[Give the <item1> to <mission_giver>]],  
          InheritPos = 1,  
          Item1Id = r2.RefId([[Client1_3594]]),  
          Item1Qty = 1,  
          Item2Id = r2.RefId([[]]),  
          Item2Qty = 0,  
          Item3Id = r2.RefId([[]]),  
          Item3Qty = 0,  
          ItemNumber = 0,  
          MissionGiver = r2.RefId([[Client1_457]]),  
          MissionSucceedText = [[Thanks a lot. I think Bobo will be feeling better now. I had made it eat the leftovers of Bastosh's last "banquet"... I hope that isn't what made it sick...]],  
          MissionText = [[I need your help! Bobo seems to be ill. I think I need a healing flower to cure it but I can't leave it on its own! Could you find one for me please?]],  
          Name = [[Mission: Cure Bobo]],  
          Repeatable = 0,  
          WaitValidationText = [[Have you found a <item1> ?]],  
          _Seed = 1154439442,  
          Behavior = {
            InstanceId = [[Client1_3592]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_3604]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_3607]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_3600]]),  
                    Action = {
                      InstanceId = [[Client1_3606]],  
                      Class = [[ActionType]],  
                      Type = [[activate]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_3605]],  
                  Class = [[EventType]],  
                  Type = [[mission asked]],  
                  Value = r2.RefId([[]])
                }
              },  
              {
                InstanceId = [[Client1_4133]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_4136]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_4110]]),  
                    Action = {
                      InstanceId = [[Client1_4135]],  
                      Class = [[ActionType]],  
                      Type = [[starts dialog]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_4134]],  
                  Class = [[EventType]],  
                  Type = [[succeeded]],  
                  Value = r2.RefId([[]])
                }
              },  
              {
                InstanceId = [[Client1_22194]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_22197]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_22189]]),  
                    Action = {
                      InstanceId = [[Client1_22196]],  
                      Class = [[ActionType]],  
                      Type = [[starts dialog]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_22195]],  
                  Class = [[EventType]],  
                  Type = [[mission asked]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
          },  
          Position = {
            InstanceId = [[Client1_3593]],  
            Class = [[Position]],  
            x = 29780,  
            y = -1385.5,  
            z = 74.5
          }
        },  
        {
          InstanceId = [[Client1_3600]],  
          Class = [[GiveItem]],  
          Active = 0,  
          Base = [[palette.entities.botobjects.bot_request_item]],  
          ContextualText = [[Take the <item1>.]],  
          InheritPos = 1,  
          Item1Id = r2.RefId([[Client1_3594]]),  
          Item1Qty = 1,  
          Item2Id = r2.RefId([[]]),  
          Item2Qty = 0,  
          Item3Id = r2.RefId([[]]),  
          Item3Qty = 0,  
          ItemNumber = 0,  
          MissionGiver = r2.RefId([[Client1_313]]),  
          MissionSucceedText = [[Don't worry, Zetonia will pay me with bodoc milk later on, hurry, the poor beast must be suffering.]],  
          MissionText = [[Bobo is ill? That greedy bodoc really eats anything he finds, I'm not surprised he is sick. Here, take this <item1> to Zetonia, she should know what to do with it.]],  
          Name = [[Gift Flower]],  
          Repeatable = 0,  
          _Seed = 1154441940,  
          Behavior = {
            InstanceId = [[Client1_3601]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
          },  
          Position = {
            InstanceId = [[Client1_3602]],  
            Class = [[Position]],  
            x = 29783.85938,  
            y = -1435.5625,  
            z = 73
          }
        },  
        {
          InstanceId = [[Client1_4110]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Dialog Zetonia End]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_4108]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_4173]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_4176]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_457]]),  
                    Action = {
                      InstanceId = [[Client1_4175]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4119]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4178]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_910]]),  
                    Action = {
                      InstanceId = [[Client1_4177]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4162]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4180]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_930]]),  
                    Action = {
                      InstanceId = [[Client1_4179]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4152]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4182]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_313]]),  
                    Action = {
                      InstanceId = [[Client1_4181]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4139]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4905]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_973]]),  
                    Action = {
                      InstanceId = [[Client1_4904]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4867]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4910]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_977]]),  
                    Action = {
                      InstanceId = [[Client1_4909]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4907]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4912]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_950]]),  
                    Action = {
                      InstanceId = [[Client1_4911]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4853]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4914]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_981]]),  
                    Action = {
                      InstanceId = [[Client1_4913]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4855]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4916]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_898]]),  
                    Action = {
                      InstanceId = [[Client1_4915]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4823]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4918]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_892]]),  
                    Action = {
                      InstanceId = [[Client1_4917]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4842]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4920]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_904]]),  
                    Action = {
                      InstanceId = [[Client1_4919]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4804]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_21755]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_1509]]),  
                    Action = {
                      InstanceId = [[Client1_21754]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_4174]],  
                  Class = [[EventType]],  
                  Type = [[end of chat]],  
                  Value = r2.RefId([[Client1_4111]])
                }
              },  
              {
                InstanceId = [[Client1_5208]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_5211]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_457]]),  
                    Action = {
                      InstanceId = [[Client1_5210]],  
                      Class = [[ActionType]],  
                      Type = [[Deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5253]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_5153]]),  
                    Action = {
                      InstanceId = [[Client1_5252]],  
                      Class = [[ActionType]],  
                      Type = [[Activate]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_5209]],  
                  Class = [[EventType]],  
                  Type = [[end of dialog]],  
                  Value = r2.RefId([[]])
                }
              },  
              {
                InstanceId = [[Client1_5526]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_5529]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_5512]]),  
                    Action = {
                      InstanceId = [[Client1_5528]],  
                      Class = [[ActionType]],  
                      Type = [[starts dialog]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_5527]],  
                  Class = [[EventType]],  
                  Type = [[end of dialog]],  
                  Value = r2.RefId([[]])
                }
              },  
              {
                InstanceId = [[Client1_5845]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_5848]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_3006]]),  
                    Action = {
                      InstanceId = [[Client1_5847]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5850]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_3017]]),  
                    Action = {
                      InstanceId = [[Client1_5849]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5852]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_3028]]),  
                    Action = {
                      InstanceId = [[Client1_5851]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5854]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_3039]]),  
                    Action = {
                      InstanceId = [[Client1_5853]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5856]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_3050]]),  
                    Action = {
                      InstanceId = [[Client1_5855]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5858]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_3056]]),  
                    Action = {
                      InstanceId = [[Client1_5857]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5860]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_3261]]),  
                    Action = {
                      InstanceId = [[Client1_5859]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_5846]],  
                  Class = [[EventType]],  
                  Type = [[start of dialog]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_4129]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_4130]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_4131]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            },  
            {
              InstanceId = [[Client1_4111]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 4,  
              Actions = {
                {
                  InstanceId = [[Client1_4112]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_4115]],  
                  Who = r2.RefId([[Client1_457]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_4113]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 5,  
              Actions = {
                {
                  InstanceId = [[Client1_4114]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_4116]],  
                  Who = r2.RefId([[Client1_457]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_5043]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 25,  
              Actions = {
                {
                  InstanceId = [[Client1_5044]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[Client1_930]]),  
                  Says = [[Client1_5047]],  
                  Who = r2.RefId([[Client1_193]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_5045]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 10,  
              Actions = {
                {
                  InstanceId = [[Client1_5046]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[Client1_930]]),  
                  Says = [[Client1_5050]],  
                  Who = r2.RefId([[Client1_193]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_5048]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 5,  
              Actions = {
                {
                  InstanceId = [[Client1_5049]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_5053]],  
                  Who = r2.RefId([[Client1_892]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_5051]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 5,  
              Actions = {
                {
                  InstanceId = [[Client1_5052]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_5056]],  
                  Who = r2.RefId([[Client1_977]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_5054]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 5,  
              Actions = {
                {
                  InstanceId = [[Client1_5055]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[Client1_930]]),  
                  Says = [[Client1_5057]],  
                  Who = r2.RefId([[Client1_193]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_5058]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 7,  
              Actions = {
                {
                  InstanceId = [[Client1_5059]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_5062]],  
                  Who = r2.RefId([[Client1_981]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_5060]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 5,  
              Actions = {
                {
                  InstanceId = [[Client1_5061]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_5065]],  
                  Who = r2.RefId([[Client1_898]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_5063]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 5,  
              Actions = {
                {
                  InstanceId = [[Client1_5064]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_5068]],  
                  Who = r2.RefId([[Client1_904]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_5066]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 5,  
              Actions = {
                {
                  InstanceId = [[Client1_5067]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_5069]],  
                  Who = r2.RefId([[Client1_457]]),  
                  WhoNoEntity = [[]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_4109]],  
            Class = [[Position]],  
            x = 29778.875,  
            y = -1387.046875,  
            z = 74.546875
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_4183]],  
          Class = [[EasterEgg]],  
          Active = 0,  
          Base = [[palette.entities.botobjects.chest_wisdom_std_sel]],  
          InheritPos = 1,  
          Item1Id = r2.RefId([[Client1_3739]]),  
          Item1Qty = 1,  
          Item2Id = r2.RefId([[]]),  
          Item2Qty = 0,  
          Item3Id = r2.RefId([[]]),  
          Item3Qty = 0,  
          ItemNumber = 3,  
          Name = [[Chest Message]],  
          _Seed = 1154529618,  
          Behavior = {
            InstanceId = [[Client1_4184]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
          },  
          Position = {
            InstanceId = [[Client1_4185]],  
            Class = [[Position]],  
            x = 29766.35938,  
            y = -1463.375,  
            z = 73
          }
        },  
        {
          InstanceId = [[Client1_5512]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Dialog Bastosh After Armas]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_5510]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_5535]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_5538]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_193]]),  
                    Action = {
                      InstanceId = [[Client1_5537]],  
                      Class = [[ActionType]],  
                      Type = [[Deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5540]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_5532]]),  
                    Action = {
                      InstanceId = [[Client1_5539]],  
                      Class = [[ActionType]],  
                      Type = [[Activate]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_5536]],  
                  Class = [[EventType]],  
                  Type = [[end of dialog]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_5513]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_5514]],  
                  Class = [[ChatAction]],  
                  Emote = [[Malevolent]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_5515]],  
                  Who = r2.RefId([[Client1_193]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_5516]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_5517]],  
                  Class = [[ChatAction]],  
                  Emote = [[Contemptuous]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_5518]],  
                  Who = r2.RefId([[Client1_193]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_5519]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 5,  
              Actions = {
                {
                  InstanceId = [[Client1_5520]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_5523]],  
                  Who = r2.RefId([[Client1_193]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_5521]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 7,  
              Actions = {
                {
                  InstanceId = [[Client1_5522]],  
                  Class = [[ChatAction]],  
                  Emote = [[Belligerent]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_5524]],  
                  Who = r2.RefId([[Client1_193]]),  
                  WhoNoEntity = [[]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_5511]],  
            Class = [[Position]],  
            x = 29768.10938,  
            y = -1465,  
            z = 73
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_6119]],  
          Class = [[ZoneTrigger]],  
          Active = 0,  
          Base = [[palette.entities.botobjects.trigger_zone]],  
          Cyclic = 0,  
          InheritPos = 1,  
          Name = [[Zone Trigger end]],  
          _Zone = [[Client1_6123]],  
          Behavior = {
            InstanceId = [[Client1_6120]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_6148]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_6151]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_6143]]),  
                    Action = {
                      InstanceId = [[Client1_6150]],  
                      Class = [[ActionType]],  
                      Type = [[activate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_21761]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_3006]]),  
                    Action = {
                      InstanceId = [[Client1_21760]],  
                      Class = [[ActionType]],  
                      Type = [[activate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_21763]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_3017]]),  
                    Action = {
                      InstanceId = [[Client1_21762]],  
                      Class = [[ActionType]],  
                      Type = [[activate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_21765]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_3028]]),  
                    Action = {
                      InstanceId = [[Client1_21764]],  
                      Class = [[ActionType]],  
                      Type = [[activate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_21767]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_3039]]),  
                    Action = {
                      InstanceId = [[Client1_21766]],  
                      Class = [[ActionType]],  
                      Type = [[activate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_21769]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_3056]]),  
                    Action = {
                      InstanceId = [[Client1_21768]],  
                      Class = [[ActionType]],  
                      Type = [[activate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_21815]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_21782]]),  
                    Action = {
                      InstanceId = [[Client1_21814]],  
                      Class = [[ActionType]],  
                      Type = [[activate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_21817]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_21801]]),  
                    Action = {
                      InstanceId = [[Client1_21816]],  
                      Class = [[ActionType]],  
                      Type = [[activate]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_6149]],  
                  Class = [[EventType]],  
                  Type = [[On Player Arrived]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_6123]],  
              Class = [[Region]],  
              Deletable = 0,  
              InheritPos = 1,  
              Name = [[Places 3]],  
              Points = {
                {
                  InstanceId = [[Client1_6125]],  
                  Class = [[RegionVertex]],  
                  Deletable = 0,  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_6126]],  
                    Class = [[Position]],  
                    x = -60.9375,  
                    y = -96.625,  
                    z = 0.0625
                  }
                },  
                {
                  InstanceId = [[Client1_23126]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_23127]],  
                    Class = [[Position]],  
                    x = 24.078125,  
                    y = -96.140625,  
                    z = 1.890625
                  }
                },  
                {
                  InstanceId = [[Client1_6128]],  
                  Class = [[RegionVertex]],  
                  Deletable = 0,  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_6129]],  
                    Class = [[Position]],  
                    x = 19.828125,  
                    y = 13.84375,  
                    z = 0.0625
                  }
                },  
                {
                  InstanceId = [[Client1_6131]],  
                  Class = [[RegionVertex]],  
                  Deletable = 0,  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_6132]],  
                    Class = [[Position]],  
                    x = -15.453125,  
                    y = 5.390625,  
                    z = -2.125
                  }
                },  
                {
                  InstanceId = [[Client1_23114]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_23115]],  
                    Class = [[Position]],  
                    x = -14.453125,  
                    y = -6.109375,  
                    z = -1.4375
                  }
                },  
                {
                  InstanceId = [[Client1_23117]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_23118]],  
                    Class = [[Position]],  
                    x = -25.09375,  
                    y = -14.625,  
                    z = -1.84375
                  }
                },  
                {
                  InstanceId = [[Client1_23120]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_23121]],  
                    Class = [[Position]],  
                    x = -45.34375,  
                    y = -10.390625,  
                    z = -3.125
                  }
                },  
                {
                  InstanceId = [[Client1_23123]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_23124]],  
                    Class = [[Position]],  
                    x = -77.359375,  
                    y = -27.21875,  
                    z = -3.046875
                  }
                },  
                {
                  InstanceId = [[Client1_6134]],  
                  Class = [[RegionVertex]],  
                  Deletable = 0,  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_6135]],  
                    Class = [[Position]],  
                    x = -80.265625,  
                    y = -53.65625,  
                    z = -1.34375
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_6122]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_6121]],  
            Class = [[Position]],  
            x = 29764.48438,  
            y = -1370.234375,  
            z = 75
          }
        },  
        {
          InstanceId = [[Client1_6143]],  
          Class = [[ChatSequence]],  
          Active = 0,  
          AutoStart = 1,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Dialog Success]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_6141]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_6144]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_6145]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_6146]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_6142]],  
            Class = [[Position]],  
            x = 29765.21875,  
            y = -1365.6875,  
            z = 75
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_718]],  
          Class = [[LootSpawner]],  
          Active = 0,  
          Base = [[palette.entities.botobjects.user_event]],  
          EasterEggId = [[Client1_721]],  
          InheritPos = 1,  
          Name = [[Mission : Shooki Hunt]],  
          Npc1Id = r2.RefId([[Client1_709]]),  
          Npc2Id = r2.RefId([[Client1_745]]),  
          Npc3Id = r2.RefId([[Client1_737]]),  
          Npc4Id = r2.RefId([[Client1_741]]),  
          Npc5Id = r2.RefId([[Client1_749]]),  
          NpcNumber = 4,  
          TriggerValue = 0,  
          _Seed = 1154080571,  
          Behavior = {
            InstanceId = [[Client1_719]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_2939]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_2942]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2776]]),  
                    Action = {
                      InstanceId = [[Client1_2941]],  
                      Class = [[ActionType]],  
                      Type = [[activate]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_2940]],  
                  Class = [[EventType]],  
                  Type = [[trigger]],  
                  Value = r2.RefId([[]])
                }
              },  
              {
                InstanceId = [[Client1_22776]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_22779]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_22771]]),  
                    Action = {
                      InstanceId = [[Client1_22778]],  
                      Class = [[ActionType]],  
                      Type = [[starts dialog]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_22777]],  
                  Class = [[EventType]],  
                  Type = [[trigger]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_721]],  
              Class = [[EasterEgg]],  
              Active = 0,  
              Base = [[palette.entities.botobjects.chest_wisdom_std_sel]],  
              InheritPos = 0,  
              Item1Id = r2.RefId([[Client1_717]]),  
              Item1Qty = 1,  
              Item2Id = r2.RefId([[]]),  
              Item2Qty = 0,  
              Item3Id = r2.RefId([[]]),  
              Item3Qty = 0,  
              ItemNumber = 3,  
              Name = [[Chest Sap]],  
              _Seed = 1154080571,  
              Behavior = {
                InstanceId = [[Client1_722]],  
                Class = [[LogicEntityBehavior]],  
                Actions = {
                }
              },  
              Components = {
              },  
              Position = {
                InstanceId = [[Client1_723]],  
                Class = [[Position]],  
                x = 29656.96875,  
                y = -1112.15625,  
                z = 74.734375
              }
            }
          },  
          Ghosts = {
          },  
          Position = {
            InstanceId = [[Client1_720]],  
            Class = [[Position]],  
            x = 29655.90625,  
            y = -1111.953125,  
            z = 74.734375
          }
        },  
        {
          InstanceId = [[Client1_1031]],  
          Class = [[RequestItem]],  
          Active = 1,  
          Base = [[palette.entities.botobjects.bot_request_item]],  
          ContextualText = [[Give the ingredients]],  
          InheritPos = 1,  
          Item1Id = r2.RefId([[Client1_717]]),  
          Item1Qty = 1,  
          Item2Id = r2.RefId([[Client1_1029]]),  
          Item2Qty = 1,  
          Item3Id = r2.RefId([[Client1_1030]]),  
          Item3Qty = 1,  
          ItemNumber = 3,  
          MissionGiver = r2.RefId([[Client1_193]]),  
          MissionSucceedText = [[Well, it's about time. Now let me cook and don't disturb me. Come back to see me once I'm done. In the meantime, you can go see Bobo the bodoc.]],  
          MissionText = [[I am Bastosh, the famous cook! I am creating a new delicacy but I am missing a few ingredients. Would you be kind enough to go get some <item1>, a <item2> and an <item3>.]],  
          Name = [[Mission: Bastosh Ingredients]],  
          Repeatable = 0,  
          WaitValidationText = [[Have you broght the ingredients I asked you for?]],  
          _Seed = 1154091554,  
          Behavior = {
            InstanceId = [[Client1_1032]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_3596]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_3599]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_3591]]),  
                    Action = {
                      InstanceId = [[Client1_3598]],  
                      Class = [[ActionType]],  
                      Type = [[activate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3731]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_1348]]),  
                    Action = {
                      InstanceId = [[Client1_3730]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_3597]],  
                  Class = [[EventType]],  
                  Type = [[succeeded]],  
                  Value = r2.RefId([[]])
                }
              },  
              {
                InstanceId = [[Client1_22049]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_22052]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_22044]]),  
                    Action = {
                      InstanceId = [[Client1_22051]],  
                      Class = [[ActionType]],  
                      Type = [[starts dialog]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_22050]],  
                  Class = [[EventType]],  
                  Type = [[mission asked]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
          },  
          Position = {
            InstanceId = [[Client1_1033]],  
            Class = [[Position]],  
            x = 29771.1875,  
            y = -1461.4375,  
            z = 73
          }
        },  
        {
          InstanceId = [[Client1_1073]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Group Clopper 1]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_1071]],  
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
          Components = {
            {
              InstanceId = [[Client1_1065]],  
              Class = [[NpcCreature]],  
              Angle = -1.783622742,  
              Base = [[palette.entities.creatures.ccbdc4]],  
              InheritPos = 1,  
              Name = [[Menacing Clopper]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1063]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_1102]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_1103]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Hunt In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_1089]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_1066]],  
                Class = [[Position]],  
                x = 29833.70313,  
                y = -1190.171875,  
                z = 61.765625
              }
            },  
            {
              InstanceId = [[Client1_1069]],  
              Class = [[NpcCreature]],  
              Angle = -1.719622731,  
              Base = [[palette.entities.creatures.ccbdc2]],  
              InheritPos = 1,  
              Name = [[Malicious Clopper]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1067]],  
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
                InstanceId = [[Client1_1070]],  
                Class = [[Position]],  
                x = 29832,  
                y = -1185.109375,  
                z = 62.390625
              }
            },  
            {
              InstanceId = [[Client1_1080]],  
              Class = [[NpcCreature]],  
              Angle = -1.635622859,  
              Base = [[palette.entities.creatures.ccbdc2]],  
              InheritPos = 1,  
              Name = [[Malicious Clopper]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1078]],  
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
                InstanceId = [[Client1_1082]],  
                Class = [[Position]],  
                x = 29836.46875,  
                y = -1186.140625,  
                z = 61.953125
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_1072]],  
            Class = [[Position]],  
            x = -7.46875,  
            y = -17.96875,  
            z = 2.140625
          }
        },  
        {
          InstanceId = [[Client1_1114]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Group Clopper 2]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_1112]],  
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
          Components = {
            {
              InstanceId = [[Client1_1106]],  
              Class = [[NpcCreature]],  
              Angle = -2.000237703,  
              Base = [[palette.entities.creatures.ccbdc4]],  
              InheritPos = 1,  
              Name = [[Menacing Clopper]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1104]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_1139]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_1140]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Hunt In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_1126]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_1107]],  
                Class = [[Position]],  
                x = 29851.42188,  
                y = -1174.53125,  
                z = 60.84375
              }
            },  
            {
              InstanceId = [[Client1_1110]],  
              Class = [[NpcCreature]],  
              Angle = -2.000237703,  
              Base = [[palette.entities.creatures.ccbdc2]],  
              InheritPos = 1,  
              Name = [[Malicious Clopper]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1108]],  
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
                InstanceId = [[Client1_1111]],  
                Class = [[Position]],  
                x = 29847.28125,  
                y = -1175.703125,  
                z = 60.953125
              }
            },  
            {
              InstanceId = [[Client1_1117]],  
              Class = [[NpcCreature]],  
              Angle = -2.000237703,  
              Base = [[palette.entities.creatures.ccbdc2]],  
              InheritPos = 1,  
              Name = [[Malicious Clopper]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1115]],  
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
                InstanceId = [[Client1_1119]],  
                Class = [[Position]],  
                x = 29852.3125,  
                y = -1177.953125,  
                z = 59.90625
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_1113]],  
            Class = [[Position]],  
            x = -1.84375,  
            y = -9.796875,  
            z = -0.859375
          }
        },  
        {
          InstanceId = [[Client1_1151]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Group Clopper 3]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_1149]],  
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
          Components = {
            {
              InstanceId = [[Client1_1143]],  
              Class = [[NpcCreature]],  
              Angle = 1.402779579,  
              Base = [[palette.entities.creatures.ccbdc4]],  
              InheritPos = 1,  
              Name = [[Menacing Clopper]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1141]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_1176]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_1177]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Hunt In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_1163]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_1144]],  
                Class = [[Position]],  
                x = 29850.07813,  
                y = -1228.140625,  
                z = 63.390625
              }
            },  
            {
              InstanceId = [[Client1_1147]],  
              Class = [[NpcCreature]],  
              Angle = 1.402779579,  
              Base = [[palette.entities.creatures.ccbdc2]],  
              InheritPos = 1,  
              Name = [[Malicious Clopper]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1145]],  
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
                InstanceId = [[Client1_1148]],  
                Class = [[Position]],  
                x = 29853.51563,  
                y = -1229.296875,  
                z = 63.03125
              }
            },  
            {
              InstanceId = [[Client1_1154]],  
              Class = [[NpcCreature]],  
              Angle = 1.402779579,  
              Base = [[palette.entities.creatures.ccbdc2]],  
              InheritPos = 1,  
              Name = [[Malicious Clopper]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1152]],  
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
                InstanceId = [[Client1_1156]],  
                Class = [[Position]],  
                x = 29847.26563,  
                y = -1226.984375,  
                z = 63.71875
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_1150]],  
            Class = [[Position]],  
            x = 0,  
            y = 0,  
            z = 0
          }
        },  
        {
          InstanceId = [[Client1_1188]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Group Clopper 4]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_1186]],  
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
          Components = {
            {
              InstanceId = [[Client1_1180]],  
              Class = [[NpcCreature]],  
              Angle = -2.872379303,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ccbdc4]],  
              InheritPos = 1,  
              Name = [[Menacing Clopper]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1178]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_1213]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_1214]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Hunt In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_1200]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_1181]],  
                Class = [[Position]],  
                x = 29875.625,  
                y = -1197.625,  
                z = 65.140625
              }
            },  
            {
              InstanceId = [[Client1_1184]],  
              Class = [[NpcCreature]],  
              Angle = -2.872379303,  
              Base = [[palette.entities.creatures.ccbdc2]],  
              InheritPos = 1,  
              Name = [[Malicious Clopper]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1182]],  
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
                InstanceId = [[Client1_1185]],  
                Class = [[Position]],  
                x = 29874.35938,  
                y = -1195.625,  
                z = 64.65625
              }
            },  
            {
              InstanceId = [[Client1_1191]],  
              Class = [[NpcCreature]],  
              Angle = -2.872379303,  
              Base = [[palette.entities.creatures.ccbdc2]],  
              InheritPos = 1,  
              Name = [[Malicious Clopper]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1189]],  
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
                InstanceId = [[Client1_1193]],  
                Class = [[Position]],  
                x = 29877.0625,  
                y = -1196.171875,  
                z = 66.328125
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_1187]],  
            Class = [[Position]],  
            x = 0,  
            y = 0,  
            z = 0
          }
        },  
        {
          InstanceId = [[Client1_1219]],  
          Class = [[BossSpawner]],  
          Active = 1,  
          Base = [[palette.entities.botobjects.user_event]],  
          BossId = r2.RefId([[Client1_1217]]),  
          Guard1Id = r2.RefId([[Client1_1073]]),  
          Guard2Id = r2.RefId([[Client1_1151]]),  
          Guard3Id = r2.RefId([[Client1_1188]]),  
          Guard4Id = r2.RefId([[Client1_1114]]),  
          Guard5Id = r2.RefId([[]]),  
          GuardNumber = 3,  
          InheritPos = 1,  
          Name = [[Spawn Clopperketh]],  
          TriggerValue = 0,  
          _Seed = 1154092783,  
          Behavior = {
            InstanceId = [[Client1_1220]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
          },  
          Ghosts = {
          },  
          Position = {
            InstanceId = [[Client1_1221]],  
            Class = [[Position]],  
            x = 29852.125,  
            y = -1211.140625,  
            z = 61
          }
        },  
        {
          InstanceId = [[Client1_1348]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Dialog Zetonia]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_1346]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_1349]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_1350]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_1351]],  
                  Who = r2.RefId([[Client1_457]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_22592]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_22593]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_23006]],  
                  Who = r2.RefId([[Client1_457]]),  
                  WhoNoEntity = [[]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_1347]],  
            Class = [[Position]],  
            x = 29778.5,  
            y = -1383.421875,  
            z = 74.453125
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_1509]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Dialog Boedix Eunix]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_1507]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_1510]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_1511]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[Client1_977]]),  
                  Says = [[Client1_23181]],  
                  Who = r2.RefId([[Client1_973]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_1513]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 5,  
              Actions = {
                {
                  InstanceId = [[Client1_1514]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_1517]],  
                  Who = r2.RefId([[Client1_977]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_1515]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 5,  
              Actions = {
                {
                  InstanceId = [[Client1_1516]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_3131]],  
                  Who = r2.RefId([[Client1_973]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_1519]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 5,  
              Actions = {
                {
                  InstanceId = [[Client1_1520]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_1521]],  
                  Who = r2.RefId([[Client1_977]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_1522]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_1523]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_1524]],  
                  Who = r2.RefId([[Client1_973]]),  
                  WhoNoEntity = [[]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_1508]],  
            Class = [[Position]],  
            x = 29785.85938,  
            y = -1453.28125,  
            z = 74.65625
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_1525]],  
          Class = [[ZoneTrigger]],  
          Active = 1,  
          Base = [[palette.entities.botobjects.trigger_zone]],  
          Cyclic = 1,  
          InheritPos = 1,  
          Name = [[Zone Trigger Dialog Boedix Eunix]],  
          _Zone = [[Client1_1529]],  
          Behavior = {
            InstanceId = [[Client1_1526]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_1543]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_1546]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_1509]]),  
                    Action = {
                      InstanceId = [[Client1_1545]],  
                      Class = [[ActionType]],  
                      Type = [[starts dialog]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_1544]],  
                  Class = [[EventType]],  
                  Type = [[On Player Arrived]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_1529]],  
              Class = [[Region]],  
              Deletable = 0,  
              InheritPos = 1,  
              Name = [[Places 1]],  
              Points = {
                {
                  InstanceId = [[Client1_1531]],  
                  Class = [[RegionVertex]],  
                  Deletable = 0,  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1532]],  
                    Class = [[Position]],  
                    x = 9.75,  
                    y = 3.53125,  
                    z = 1.515625
                  }
                },  
                {
                  InstanceId = [[Client1_1534]],  
                  Class = [[RegionVertex]],  
                  Deletable = 0,  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1535]],  
                    Class = [[Position]],  
                    x = -3.0625,  
                    y = 2.1875,  
                    z = -0.609375
                  }
                },  
                {
                  InstanceId = [[Client1_1537]],  
                  Class = [[RegionVertex]],  
                  Deletable = 0,  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1538]],  
                    Class = [[Position]],  
                    x = -6.75,  
                    y = -5.46875,  
                    z = -0.765625
                  }
                },  
                {
                  InstanceId = [[Client1_1540]],  
                  Class = [[RegionVertex]],  
                  Deletable = 0,  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1541]],  
                    Class = [[Position]],  
                    x = -0.984375,  
                    y = -11.609375,  
                    z = 0.265625
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_1528]],  
                Class = [[Position]],  
                x = -0.40625,  
                y = 0.75,  
                z = -0.015625
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_1527]],  
            Class = [[Position]],  
            x = 29779.64063,  
            y = -1450.859375,  
            z = 73
          }
        },  
        {
          InstanceId = [[Client1_1599]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Dialog Tu'Rlin]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_1597]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_3255]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_3258]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_1188]]),  
                    Action = {
                      InstanceId = [[Client1_3257]],  
                      Class = [[ActionType]],  
                      Type = [[Activate]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_3256]],  
                  Class = [[EventType]],  
                  Type = [[end of dialog]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_1600]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_1601]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_23128]],  
                  Who = r2.RefId([[Client1_313]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_22899]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 5,  
              Actions = {
                {
                  InstanceId = [[Client1_22900]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_22901]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_1598]],  
            Class = [[Position]],  
            x = 29784.32813,  
            y = -1432.015625,  
            z = 73.765625
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_1628]],  
          Class = [[GiveItem]],  
          Active = 0,  
          Base = [[palette.entities.botobjects.bot_request_item]],  
          ContextualText = [[Accept the <item1>.]],  
          InheritPos = 1,  
          Item1Id = r2.RefId([[Client1_1029]]),  
          Item1Qty = 1,  
          Item2Id = r2.RefId([[]]),  
          Item2Qty = 0,  
          Item3Id = r2.RefId([[]]),  
          Item3Qty = 0,  
          ItemNumber = 3,  
          MissionGiver = r2.RefId([[Client1_313]]),  
          MissionSucceedText = [[Take it, but be careful, it has strange properties if eaten.]],  
          MissionText = [[What? You have seen the big blue clopper? So... the legend IS true after all! Here, you deserve this <item1>.]],  
          Name = [[Gift Mushroom]],  
          Repeatable = 0,  
          _Seed = 1154336470,  
          Behavior = {
            InstanceId = [[Client1_1629]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
          },  
          Position = {
            InstanceId = [[Client1_1630]],  
            Class = [[Position]],  
            x = 29784.17188,  
            y = -1430.28125,  
            z = 73
          }
        },  
        {
          InstanceId = [[Client1_2597]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Dialog Sap 1]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_2595]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_2650]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_2653]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_718]]),  
                    Action = {
                      InstanceId = [[Client1_2652]],  
                      Class = [[ActionType]],  
                      Type = [[activate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_2858]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_737]]),  
                    Action = {
                      InstanceId = [[Client1_2857]],  
                      Class = [[ActionType]],  
                      Type = [[Activate]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_2651]],  
                  Class = [[EventType]],  
                  Type = [[end of dialog]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_2603]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_2604]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_2608]],  
                  Who = r2.RefId([[Client1_679]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_2606]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 5,  
              Actions = {
                {
                  InstanceId = [[Client1_2607]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_2609]],  
                  Who = r2.RefId([[Client1_679]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_2610]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_2611]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[Client1_679]]),  
                  Says = [[Client1_2612]],  
                  Who = r2.RefId([[Client1_683]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_2613]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 5,  
              Actions = {
                {
                  InstanceId = [[Client1_2614]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[Client1_683]]),  
                  Says = [[Client1_2615]],  
                  Who = r2.RefId([[Client1_679]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_2616]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 5,  
              Actions = {
                {
                  InstanceId = [[Client1_2617]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[Client1_457]]),  
                  Says = [[Client1_2620]],  
                  Who = r2.RefId([[Client1_683]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_2618]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_2619]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_2621]],  
                  Who = r2.RefId([[Client1_683]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_2622]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 5,  
              Actions = {
                {
                  InstanceId = [[Client1_2623]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[Client1_457]]),  
                  Says = [[Client1_2626]],  
                  Who = r2.RefId([[Client1_679]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_2624]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 5,  
              Actions = {
                {
                  InstanceId = [[Client1_2625]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_2627]],  
                  Who = r2.RefId([[Client1_683]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_22830]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_22831]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_22832]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_2596]],  
            Class = [[Position]],  
            x = 29652.4375,  
            y = -1113.421875,  
            z = 74.859375
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_21223]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 1,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Dialog Start]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_21221]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_21224]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_21225]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_21229]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            },  
            {
              InstanceId = [[Client1_21227]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_21228]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_21230]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_21222]],  
            Class = [[Position]],  
            x = 29764.26563,  
            y = -1437.734375,  
            z = 74.796875
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_21735]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 1,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Dialog Bastosh Start]],  
          Repeating = 1,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_21733]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_21736]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_21737]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_21740]],  
                  Who = r2.RefId([[Client1_193]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_21738]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_21739]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_21741]],  
                  Who = r2.RefId([[Client1_193]]),  
                  WhoNoEntity = [[]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_21734]],  
            Class = [[Position]],  
            x = 29768.4375,  
            y = -1463.734375,  
            z = 73.671875
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_21782]],  
          Class = [[ChatSequence]],  
          Active = 0,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Dialog Guedon End]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_21780]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_21783]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_21784]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_21785]],  
                  Who = r2.RefId([[Client1_950]]),  
                  WhoNoEntity = [[]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_21781]],  
            Class = [[Position]],  
            x = 29760.92188,  
            y = -1379.625,  
            z = 74.96875
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_21801]],  
          Class = [[ChatSequence]],  
          Active = 0,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Dialog Lyrius End]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_21799]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_21802]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_21803]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_21809]],  
                  Who = r2.RefId([[Client1_981]]),  
                  WhoNoEntity = [[]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_21800]],  
            Class = [[Position]],  
            x = 29762.45313,  
            y = -1379.640625,  
            z = 74.953125
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_22044]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Broadcast Mission Bastosh]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_22042]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_22045]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_22046]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_22047]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_22043]],  
            Class = [[Position]],  
            x = 29774.35938,  
            y = -1462.3125,  
            z = 73.9375
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_22189]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Broadcast Mission Zetonia]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_22187]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_22190]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_22191]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_22192]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_22188]],  
            Class = [[Position]],  
            x = 29779.0625,  
            y = -1384.453125,  
            z = 74.484375
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_22378]],  
          Class = [[ZoneTrigger]],  
          Active = 1,  
          Base = [[palette.entities.botobjects.trigger_zone]],  
          Cyclic = 0,  
          InheritPos = 1,  
          Name = [[Zone Trigger Egg]],  
          _Zone = [[Client1_22382]],  
          Behavior = {
            InstanceId = [[Client1_22379]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_22417]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_22420]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_22413]]),  
                    Action = {
                      InstanceId = [[Client1_22419]],  
                      Class = [[ActionType]],  
                      Type = [[Activate]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_22418]],  
                  Class = [[EventType]],  
                  Type = [[On Player Arrived]],  
                  Value = r2.RefId([[]])
                }
              },  
              {
                InstanceId = [[Client1_22422]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_22425]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_22413]]),  
                    Action = {
                      InstanceId = [[Client1_22424]],  
                      Class = [[ActionType]],  
                      Type = [[Deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_22423]],  
                  Class = [[EventType]],  
                  Type = [[On Player Left]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_22382]],  
              Class = [[Region]],  
              Deletable = 0,  
              InheritPos = 1,  
              Name = [[Places 5]],  
              Points = {
                {
                  InstanceId = [[Client1_22384]],  
                  Class = [[RegionVertex]],  
                  Deletable = 0,  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_22385]],  
                    Class = [[Position]],  
                    x = 6.875,  
                    y = 1.109375,  
                    z = 1.078125
                  }
                },  
                {
                  InstanceId = [[Client1_22396]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_22397]],  
                    Class = [[Position]],  
                    x = 4.84375,  
                    y = 5.015625,  
                    z = 1.109375
                  }
                },  
                {
                  InstanceId = [[Client1_22387]],  
                  Class = [[RegionVertex]],  
                  Deletable = 0,  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_22388]],  
                    Class = [[Position]],  
                    x = 1.890625,  
                    y = 7.140625,  
                    z = 1.09375
                  }
                },  
                {
                  InstanceId = [[Client1_22399]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_22400]],  
                    Class = [[Position]],  
                    x = -4.625,  
                    y = 5.359375,  
                    z = -1.46875
                  }
                },  
                {
                  InstanceId = [[Client1_22390]],  
                  Class = [[RegionVertex]],  
                  Deletable = 0,  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_22391]],  
                    Class = [[Position]],  
                    x = -8.1875,  
                    y = -0.9375,  
                    z = -1.359375
                  }
                },  
                {
                  InstanceId = [[Client1_22405]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_22406]],  
                    Class = [[Position]],  
                    x = -5.96875,  
                    y = -5.359375,  
                    z = 0.65625
                  }
                },  
                {
                  InstanceId = [[Client1_22393]],  
                  Class = [[RegionVertex]],  
                  Deletable = 0,  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_22394]],  
                    Class = [[Position]],  
                    x = -1.4375,  
                    y = -7.671875,  
                    z = 0.65625
                  }
                },  
                {
                  InstanceId = [[Client1_22402]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_22403]],  
                    Class = [[Position]],  
                    x = 3.328125,  
                    y = -7.171875,  
                    z = 0.390625
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_22381]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_22380]],  
            Class = [[Position]],  
            x = 29509.21875,  
            y = -1526.65625,  
            z = 76.3125
          }
        },  
        {
          InstanceId = [[Client1_22410]],  
          Class = [[EasterEgg]],  
          Active = 0,  
          Angle = 0,  
          Base = [[palette.entities.botobjects.chest_wisdom_std_sel]],  
          InheritPos = 1,  
          Item1Id = r2.RefId([[Client1_1030]]),  
          Item1Qty = 1,  
          Item2Id = r2.RefId([[]]),  
          Item2Qty = 0,  
          Item3Id = r2.RefId([[]]),  
          Item3Qty = 0,  
          ItemNumber = 0,  
          Name = [[Chest Egg]],  
          _Seed = 1158677402,  
          Behavior = {
            InstanceId = [[Client1_22411]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
          },  
          Position = {
            InstanceId = [[Client1_22412]],  
            Class = [[Position]],  
            x = 29509.45313,  
            y = -1528.328125,  
            z = 76.328125
          }
        },  
        {
          InstanceId = [[Client1_22413]],  
          Class = [[Timer]],  
          Active = 0,  
          Base = [[palette.entities.botobjects.timer]],  
          Cyclic = 0,  
          InheritPos = 1,  
          Minutes = 0,  
          Name = [[Timer Egg]],  
          Secondes = 3,  
          Behavior = {
            InstanceId = [[Client1_22414]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_22427]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_22430]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_22410]]),  
                    Action = {
                      InstanceId = [[Client1_22429]],  
                      Class = [[ActionType]],  
                      Type = [[activate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_22441]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_22433]]),  
                    Action = {
                      InstanceId = [[Client1_22440]],  
                      Class = [[ActionType]],  
                      Type = [[starts dialog]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_22428]],  
                  Class = [[EventType]],  
                  Type = [[On Trigger]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
          },  
          Position = {
            InstanceId = [[Client1_22415]],  
            Class = [[Position]],  
            x = 29509.34375,  
            y = -1525.25,  
            z = 76.3125
          }
        },  
        {
          InstanceId = [[Client1_22433]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Broadcast Egg Pop]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_22431]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_22434]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_22435]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_23004]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_22432]],  
            Class = [[Position]],  
            x = 29507.57813,  
            y = -1529.53125,  
            z = 76.140625
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_22755]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Broadcast Chest Pop Message]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_22753]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_22756]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_22757]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_22758]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_22754]],  
            Class = [[Position]],  
            x = 29764.75,  
            y = -1466.203125,  
            z = 73.75
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_22763]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Broadcast  Blue Clopper]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_22761]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_22764]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_22765]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_22766]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_22762]],  
            Class = [[Position]],  
            x = 29841.70313,  
            y = -1207.703125,  
            z = 62.53125
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_22771]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Broadcast Sap Pop]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_22769]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_22772]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_22773]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_22774]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_22770]],  
            Class = [[Position]],  
            x = 29655.54688,  
            y = -1115.140625,  
            z = 74.90625
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_5757]],  
          Class = [[ChatSequence]],  
          Active = 0,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Dialog O'Darghan End]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_5755]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_5798]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_6118]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_930]]),  
                    Action = {
                      InstanceId = [[Client1_6117]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_946]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_6116]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_910]]),  
                    Action = {
                      InstanceId = [[Client1_6115]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_926]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_6114]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_981]]),  
                    Action = {
                      InstanceId = [[Client1_6113]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_1000]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_6112]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_950]]),  
                    Action = {
                      InstanceId = [[Client1_6111]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_969]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_6110]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_904]]),  
                    Action = {
                      InstanceId = [[Client1_6109]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_906]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_6108]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_898]]),  
                    Action = {
                      InstanceId = [[Client1_6107]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_900]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_6106]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_892]]),  
                    Action = {
                      InstanceId = [[Client1_6105]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_894]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5843]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_457]]),  
                    Action = {
                      InstanceId = [[Client1_5842]],  
                      Class = [[ActionType]],  
                      Type = [[Activate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5841]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_904]]),  
                    Action = {
                      InstanceId = [[Client1_5840]],  
                      Class = [[ActionType]],  
                      Type = [[Activate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5839]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_892]]),  
                    Action = {
                      InstanceId = [[Client1_5838]],  
                      Class = [[ActionType]],  
                      Type = [[Activate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5837]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_313]]),  
                    Action = {
                      InstanceId = [[Client1_5836]],  
                      Class = [[ActionType]],  
                      Type = [[Activate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5835]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_898]]),  
                    Action = {
                      InstanceId = [[Client1_5834]],  
                      Class = [[ActionType]],  
                      Type = [[Activate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5833]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_981]]),  
                    Action = {
                      InstanceId = [[Client1_5832]],  
                      Class = [[ActionType]],  
                      Type = [[Activate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5831]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_950]]),  
                    Action = {
                      InstanceId = [[Client1_5830]],  
                      Class = [[ActionType]],  
                      Type = [[Activate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5829]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_977]]),  
                    Action = {
                      InstanceId = [[Client1_5828]],  
                      Class = [[ActionType]],  
                      Type = [[Activate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5827]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_930]]),  
                    Action = {
                      InstanceId = [[Client1_5826]],  
                      Class = [[ActionType]],  
                      Type = [[Activate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5825]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_910]]),  
                    Action = {
                      InstanceId = [[Client1_5824]],  
                      Class = [[ActionType]],  
                      Type = [[Activate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5823]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_973]]),  
                    Action = {
                      InstanceId = [[Client1_5822]],  
                      Class = [[ActionType]],  
                      Type = [[Activate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5821]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_5153]]),  
                    Action = {
                      InstanceId = [[Client1_5820]],  
                      Class = [[ActionType]],  
                      Type = [[Deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5819]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_5181]]),  
                    Action = {
                      InstanceId = [[Client1_5818]],  
                      Class = [[ActionType]],  
                      Type = [[Deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5817]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_5195]]),  
                    Action = {
                      InstanceId = [[Client1_5816]],  
                      Class = [[ActionType]],  
                      Type = [[Deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5815]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_5146]]),  
                    Action = {
                      InstanceId = [[Client1_5814]],  
                      Class = [[ActionType]],  
                      Type = [[Deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5813]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_5188]]),  
                    Action = {
                      InstanceId = [[Client1_5812]],  
                      Class = [[ActionType]],  
                      Type = [[Deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5811]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_5167]]),  
                    Action = {
                      InstanceId = [[Client1_5810]],  
                      Class = [[ActionType]],  
                      Type = [[Deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5809]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_5202]]),  
                    Action = {
                      InstanceId = [[Client1_5808]],  
                      Class = [[ActionType]],  
                      Type = [[Deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5807]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_5139]]),  
                    Action = {
                      InstanceId = [[Client1_5806]],  
                      Class = [[ActionType]],  
                      Type = [[Deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5805]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_5160]]),  
                    Action = {
                      InstanceId = [[Client1_5804]],  
                      Class = [[ActionType]],  
                      Type = [[Deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5803]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_5174]]),  
                    Action = {
                      InstanceId = [[Client1_5802]],  
                      Class = [[ActionType]],  
                      Type = [[Deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5801]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_5133]]),  
                    Action = {
                      InstanceId = [[Client1_5800]],  
                      Class = [[ActionType]],  
                      Type = [[Deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_5799]],  
                  Class = [[EventType]],  
                  Type = [[end of dialog]],  
                  Value = r2.RefId([[]])
                }
              },  
              {
                InstanceId = [[Client1_6137]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_6140]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_6119]]),  
                    Action = {
                      InstanceId = [[Client1_6139]],  
                      Class = [[ActionType]],  
                      Type = [[activate]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_6138]],  
                  Class = [[EventType]],  
                  Type = [[end of dialog]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_5758]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_5759]],  
                  Class = [[ChatAction]],  
                  Emote = [[Grin]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_23180]],  
                  Who = r2.RefId([[Client1_683]]),  
                  WhoNoEntity = [[]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_5756]],  
            Class = [[Position]],  
            x = 29659.0625,  
            y = -1106.375,  
            z = 74.296875
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_23052]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[ Group goaris nid]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_23050]],  
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
          Components = {
            {
              InstanceId = [[Client1_23031]],  
              Class = [[NpcCreature]],  
              Angle = 0.09719523787,  
              Base = [[palette.entities.creatures.cccdc4]],  
              InheritPos = 1,  
              Name = [[Menacing Goari]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_23029]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_23053]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_23054]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Rest In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_22623]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      },  
                      {
                        InstanceId = [[Client1_23129]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Hunt In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_22343]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[90]],  
                        Type = [[None]],  
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
                InstanceId = [[Client1_23032]],  
                Class = [[Position]],  
                x = 29516.75,  
                y = -1516.390625,  
                z = 79.640625
              }
            },  
            {
              InstanceId = [[Client1_23048]],  
              Class = [[NpcCreature]],  
              Angle = 0.5434116721,  
              Base = [[palette.entities.creatures.cccdc4]],  
              InheritPos = 1,  
              Name = [[Menacing Goari]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_23046]],  
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
                InstanceId = [[Client1_23049]],  
                Class = [[Position]],  
                x = 29514.57813,  
                y = -1535,  
                z = 75.84375
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_23051]],  
            Class = [[Position]],  
            x = 0,  
            y = 0,  
            z = 0
          }
        }
      },  
      Position = {
        InstanceId = [[Client1_115]],  
        Class = [[Position]],  
        x = 0,  
        y = 0,  
        z = 0
      }
    }
  },  
  Behavior = {
    InstanceId = [[Client1_107]],  
    Class = [[LogicEntityBehavior]],  
    Actions = {
    }
  },  
  Description = {
    InstanceId = [[Client1_105]],  
    Class = [[MapDescription]],  
    Creator = [[Ring(Nevrax)]],  
    LevelId = 2,  
    OptimalNumberOfPlayer = 0,  
    ShortDescription = [[Come and try Bastosh's delicious creations.]],  
    Title = [[Bastosh, the Fyros cook.]]
  },  
  Locations = {
    {
      InstanceId = [[Client1_118]],  
      Class = [[Location]],  
      EntryPoint = [[uiR2EntryPoint05]],  
      IslandName = [[uiR2_Deserts11]],  
      ManualSeason = 1,  
      Name = [[Blue Oasis (Desert 17)]],  
      Season = [[Spring]],  
      ShortDescription = [[]],  
      Time = 0
    }
  },  
  PlotItems = {
    {
      InstanceId = [[Client1_717]],  
      Class = [[PlotItem]],  
      Comment = [[]],  
      Desc = [[Sap which was foraged in the botoga forest.]],  
      Name = [[botoga sap]],  
      SheetId = 8635950
    },  
    {
      InstanceId = [[Client1_1029]],  
      Class = [[PlotItem]],  
      Comment = [[]],  
      Desc = [[Are you sure it is edible?]],  
      Name = [[mushroom]],  
      SheetId = 8632366
    },  
    {
      InstanceId = [[Client1_1030]],  
      Class = [[PlotItem]],  
      Comment = [[]],  
      Desc = [[One day, a Goari will hatch from this egg... unless someone cooks it!]],  
      Name = [[egg]],  
      SheetId = 8670766
    },  
    {
      InstanceId = [[Client1_3594]],  
      Class = [[PlotItem]],  
      Comment = [[]],  
      Desc = [[Flower with healing properties which can cure Bobo the bodoc.]],  
      Name = [[healing flower]],  
      SheetId = 8644654
    },  
    {
      InstanceId = [[Client1_3739]],  
      Class = [[PlotItem]],  
      Comment = [[]],  
      Desc = [[O'Darghan's message for Bastosh, giving her a secret recipe and asking her to be careful when using it.]],  
      Name = [[O'Darghan's message]],  
      SheetId = 8664110
    }
  },  
  Position = {
    InstanceId = [[Client1_108]],  
    Class = [[Position]],  
    x = 0,  
    y = 0,  
    z = 0
  },  
  Texts = {
    InstanceId = [[Client1_106]],  
    Class = [[TextManager]],  
    Texts = {
      {
        InstanceId = [[Client1_3059]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Tu'Rlin is obsessed with this legend about a blue Clopper. Nobody here has ever seen it...we go out of our way to avoid the Clopper nest to the east of the village.]]
      },  
      {
        InstanceId = [[Client1_3131]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[If that were the only problem...but to boot, she always wants everybody else to do her errands for her since she's supposedly too busy creating!]]
      },  
      {
        InstanceId = [[Client1_3332]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[I'm not so sure that Bastosh practices good hygiene. I saw another dead body behind her tent...one that must have been there for a while already!]]
      },  
      {
        InstanceId = [[Client1_3516]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Well done! You have literally rid the forest of the Shooki.]]
      },  
      {
        InstanceId = [[Client1_4115]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Ha, it looks like Bastosh's new dish is ready. Just to make her happy we'll go but I'm sure nobody really feels like tasting it...]]
      },  
      {
        InstanceId = [[Client1_4116]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[You're not from the village: don't feel like you have to come.]]
      },  
      {
        InstanceId = [[Client1_4131]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Bastosh: Come on and savour the delicious new recipe prepared by Bastosh the magnificent!]]
      },  
      {
        InstanceId = [[Client1_5047]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Come one, come all! Come taste my marvellous new dish! ]]
      },  
      {
        InstanceId = [[Client1_5050]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[To celebrate my latest creation, I'm offering all of you a free taste.]]
      },  
      {
        InstanceId = [[Client1_5053]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Now that's what you'd call nice. Maybe Bastosh is nicer than we ever gave her credit for.]]
      },  
      {
        InstanceId = [[Client1_5056]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Ha! that's what you think! If it's just so I'm as sick as a dog for the next three days, no thanks!]]
      },  
      {
        InstanceId = [[Client1_5057]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[My friends, now that I have you all together, I'd like to have you taste my succulent casserole. Go ahead and enjoy!]]
      },  
      {
        InstanceId = [[Client1_5062]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Hmm, it's actually not so bad...]]
      },  
      {
        InstanceId = [[Client1_5065]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[In fact, you could almost say it's good...]]
      },  
      {
        InstanceId = [[Client1_5068]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[True...but... suddenly, I don't feel so well...]]
      },  
      {
        InstanceId = [[Client1_5069]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Yeah, me neither, but...]]
      },  
      {
        InstanceId = [[Client1_5515]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Ha ha ha ha ha!]]
      },  
      {
        InstanceId = [[Client1_5518]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[You spineless Kitins! You are going to be a bit less proud now!]]
      },  
      {
        InstanceId = [[Client1_5523]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[All of you always made fun of my cooking - I've finally gotten back at you! Now you're nothing more than stinking Armas, good for nothing else than throwing into one of my stews!]]
      },  
      {
        InstanceId = [[Client1_5524]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Now, all I need to do is thank all of you who helped me gather up the ingredients...ha ha ha ha ha !]]
      },  
      {
        InstanceId = [[Client1_5729]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Mais....que font ces armas ici ??? Et où sont passés les villageois ?]]
      },  
      {
        InstanceId = [[Client1_5734]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Quoi ?! C'est Bastosh qui les a mis dans cet état ? Il faut faire quelque chose !]]
      },  
      {
        InstanceId = [[Client1_5735]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Allez voir O'Darghan, peut être pourra-t-il faire quelque chose. Son grand-père était un mage Tryker, si quelqu'un peut résoudre ce problème c'est lui.]]
      },  
      {
        InstanceId = [[Client1_6146]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Congratulations! You have managed to rid the village of Bastosh and save all the inhabitants!]]
      },  
      {
        InstanceId = [[Client1_781]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Bienvenue au village de Menos. Vous êtes à la porte du pachyderme.]]
      },  
      {
        InstanceId = [[Client1_814]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Bienvenue au village de Menos. Vous etes à la porte du pachyderme.]]
      },  
      {
        InstanceId = [[Client1_1334]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Bienvenue au village de Menos. Vous êtes à la porte de l'Igara]]
      },  
      {
        InstanceId = [[Client1_1335]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Bienvenue au village de Menos. Vous êtes à la porte de l'Igara.]]
      },  
      {
        InstanceId = [[Client1_1351]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Hello! I am Zetonia, Bodoc trainer, and this is Bobo.]]
      },  
      {
        InstanceId = [[Client1_1517]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[You're right! She always has to remind us that she is a great chef.]]
      },  
      {
        InstanceId = [[Client1_1521]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[The worst of it is that she is the only one who appreciates her cooking!]]
      },  
      {
        InstanceId = [[Client1_1524]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Ha, you make me feel better. I thought I was the only one who didn't appreciate her cooking. Nobody ever says anything to her.]]
      },  
      {
        InstanceId = [[Client1_2608]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[We are O'Darghan and Dekos, Master Extractors of the Botogas Forest in the village of Menos.]]
      },  
      {
        InstanceId = [[Client1_2609]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Would you like some sap?]]
      },  
      {
        InstanceId = [[Client1_2612]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Wait a moment....Dekos, are you thinking what I'm thinking?]]
      },  
      {
        InstanceId = [[Client1_2615]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[If what you are thinking has something to do with our problem, then yes, we're thinking the same thing.]]
      },  
      {
        InstanceId = [[Client1_2620]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Listen to me, you who came to ask us if we had some sap.]]
      },  
      {
        InstanceId = [[Client1_2621]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[We are going to give you what you want, but first we ask that you do us a small favour.]]
      },  
      {
        InstanceId = [[Client1_2626]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Our forest is starting to be overrun by Shookis and we can't seem to get rid of them.]]
      },  
      {
        InstanceId = [[Client1_2627]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[If you could manage to destroy all the Shookis in the Botogas forest, we'll give you the best sap to be found on Atys.]]
      },  
      {
        InstanceId = [[Client1_2638]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Greetings.]]
      },  
      {
        InstanceId = [[Client1_2927]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[We are forever in your debt. Without you, the Botogas would have perished.]]
      },  
      {
        InstanceId = [[Client1_2928]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Here, this chest contains the sap. And again, our heartfelt thanks.]]
      },  
      {
        InstanceId = [[Client1_3012]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Do you know that there is a forest of Botogas north of the village? I played there often as a child.]]
      },  
      {
        InstanceId = [[Client1_3020]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Dekos and O'Darghan are our two sap extractors. They are nice but not very brave.]]
      },  
      {
        InstanceId = [[Client1_3047]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Tu'Rlin is really extraordinary! He knows everything there is to know about the animal and plant worlds. He is an expert on mushrooms.]]
      },  
      {
        InstanceId = [[Client1_3053]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[I don't know what Bastosh is whipping up for us but I sure hope it's better than the last time....I was sick for 3 days!]]
      },  
      {
        InstanceId = [[Client1_21229]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Welcome to the Fyros village of Botacus.]]
      },  
      {
        InstanceId = [[Client1_21230]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Visit the lake, the Botogas forest and don't forget to taste the delicious creations of the great cook Bastosh!]]
      },  
      {
        InstanceId = [[Client1_21740]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Hey, you over there!]]
      },  
      {
        InstanceId = [[Client1_21741]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Hey you! Come here!]]
      },  
      {
        InstanceId = [[Client1_21785]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Finally, it's rather fun to be an Arma....too bad I couldn't stay that way a little longer...]]
      },  
      {
        InstanceId = [[Client1_21809]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Never again will I hunt the Armas. I might even go ahead and adopt one...]]
      },  
      {
        InstanceId = [[Client1_22047]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Mission: Bring back a mushroom, an Igara's egg and some Botoga sap to Bastosh.]]
      },  
      {
        InstanceId = [[Client1_22124]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Mission : Trouver et cibler le grand cloppeur bleu]]
      },  
      {
        InstanceId = [[Client1_22192]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Mission: Find a medical flower for Bobo and bring it back to Zetonia.]]
      },  
      {
        InstanceId = [[Client1_22203]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Mission : Tuez tous les Shookis de la forêt pour obtenir de la sève]]
      },  
      {
        InstanceId = [[Client1_22758]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[You have killed Bastosh. You should search her tent to find out how to save the villagers.]]
      },  
      {
        InstanceId = [[Client1_22766]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[You've found it! The gigantic blue Clopper really does exist! You should tell Tu'Rlin about it.]]
      },  
      {
        InstanceId = [[Client1_22774]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Congratulations! You have destroyed all the Shookis in the forest! Go get your reward.]]
      },  
      {
        InstanceId = [[Client1_22832]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Mission: Kill all the Shookis in the forest to get the sap.]]
      },  
      {
        InstanceId = [[Client1_22901]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Mission : Find and target the big blue Clopper.]]
      },  
      {
        InstanceId = [[Client1_23004]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[You have just discovered a Goaris nest containing some eggs.]]
      },  
      {
        InstanceId = [[Client1_23005]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Watch out. The Goaris to the west of the village are extremely aggressive right now. They must have eggs to protect...]]
      },  
      {
        InstanceId = [[Client1_23006]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[The Goaris bury their eggs at the base of little Savaniel trees that can be found in the desert. If you want any, that is where you should look.]]
      },  
      {
        InstanceId = [[Client1_23128]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[I am Tu'Rlin, supplier of raw material and leading mushroom specialist.{break}In fact, I have a magnficent specimen here to show you. {break}What? You're interested? Unfortunately, it's not for sale. We might be able to work something out however...{break}Legend has it that a gigantic blue Clopper lives in the dunes to the north of the village.{break}I've always wanted to know if this legend was really true... but I'm too much of a scaredy cat to go look for it myself.{break}If you manage to solve the mystery of this gigantic blue Clopper, I'll be happy to offer you this extremely rare mushroom.]]
      },  
      {
        InstanceId = [[Client1_23180]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Ha ha ha ha ha! So that's why Bastosh asked me for the transformation recipe!{break}That poor fool knew that my grandfather was a Tryker medicine man. She came to ask if I knew any secret recipes.{break}I didn't know that she wanted to get back at the village residents. But don't worry, what she didn't know is that the effects don't last long.{break}I think that by the time you get back, the villagers will all have recovered.{break}This story ends badly for Bastosh but at least she won't be giving food poisoning to the village any more with her revolting meals.{break}We just have to hope that Ma'Duk will bring her back to life somewhere she can learn to cook correctly...]]
      },  
      {
        InstanceId = [[Client1_23181]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[That Bastosh...who does she think she is?]]
      }
    }
  },  
  UserComponents = {
  },  
  Versions = {
    Act = 6,  
    ActionStep = 2,  
    ActionType = 0,  
    ActivitySequence = 0,  
    ActivityStep = 1,  
    Behavior = 1,  
    BossSpawner = 1,  
    ChatAction = 0,  
    ChatSequence = 1,  
    ChatStep = 0,  
    ConditionStep = 0,  
    ConditionType = 0,  
    DefaultFeature = 0,  
    EasterEgg = 1,  
    EventType = 0,  
    GiveItem = 1,  
    Location = 1,  
    LogicEntityAction = 0,  
    LogicEntityBehavior = 1,  
    LootSpawner = 1,  
    MapDescription = 0,  
    Npc = 1,  
    NpcCreature = 1,  
    NpcCustom = 1,  
    NpcGrpFeature = 1,  
    NpcPlant = 1,  
    PlotItem = 0,  
    Position = 0,  
    Region = 1,  
    RegionVertex = 1,  
    RequestItem = 1,  
    Road = 1,  
    Scenario = 4,  
    TextManager = 0,  
    TextManagerEntry = 0,  
    Timer = 1,  
    WayPoint = 1,  
    ZoneTrigger = 1
  }
}