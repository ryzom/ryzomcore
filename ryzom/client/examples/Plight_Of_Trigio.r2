---- Header
-- Version = '1'
-- Signature = 'DEV:2f80892713f0ec079967a62fe990c1f4'
-- HeaderMD5 = '98cfcfa607e476fd5d3d677b70a6d61d'
-- BodyMD5 = '63b9b4b215c3681ed41968ff8c85839a'
-- Title = 'Trigio's Plight'
-- Name = 'Plight_Of_Trigio.r2'
-- ShortDescription = 'A farmer's desperate flee from the Kitin, help him get to safety!'
-- FirstLocation = 'uiR2_Forest35'
-- RingPointLevel = 'a1:p13:j7:l6:d7:f7'
-- CreateBy = 'Calebrien'
-- CreatorMD5 = '93B71DDF'
-- CreationDate = '10/17/06 16:39:31'
-- ModifiedBy = 'Calebrien'
-- ModifierMD5 = '93B71DDF'
-- OtherCharAccess = 'Full'
-- ModificationDate = '10/17/06 16:39:31'
-- Rules = 'Masterless'
-- Level = '200'
-- Type = 'so_story_telling'
-- Language = 'en'
-- InitialIsland = 'uiR2_Forest35'
-- InitialEntryPoint = 'uiR2EntryPoint02'
-- InitialSeason = 'Spring'
---- /Header

scenario = {
  InstanceId = [[Client1_5]],  
  Class = [[Scenario]],  
  AccessRules = [[liberal]],  
  InheritPos = 1,  
  Name = [[Plight_Of_Trigio.r2]],  
  VersionName = [[Vianney Rocks]],  
  Acts = {
    {
      InstanceId = [[Client1_8]],  
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
        InstanceId = [[Client1_6]],  
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
            {
              InstanceId = [[Client1_63]],  
              Class = [[Npc]],  
              Angle = -2.75,  
              Base = [[palette.entities.botobjects.campfire]],  
              InheritPos = 1,  
              Name = [[camp fire 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_61]],  
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
                InstanceId = [[Client1_64]],  
                Class = [[Position]],  
                x = 27080.95313,  
                y = -12939.64063,  
                z = -19.078125
              }
            },  
            {
              InstanceId = [[Client1_67]],  
              Class = [[Npc]],  
              Angle = -2.71875,  
              Base = [[palette.entities.botobjects.jar_3]],  
              InheritPos = 1,  
              Name = [[3 jars 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_65]],  
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
                InstanceId = [[Client1_68]],  
                Class = [[Position]],  
                x = 27081.57813,  
                y = -12936.875,  
                z = -18.96875
              }
            },  
            {
              InstanceId = [[Client1_71]],  
              Class = [[Npc]],  
              Angle = -2.53125,  
              Base = [[palette.entities.botobjects.chest_old]],  
              InheritPos = 1,  
              Name = [[old chest 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_69]],  
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
                InstanceId = [[Client1_72]],  
                Class = [[Position]],  
                x = 27080.35938,  
                y = -12936.84375,  
                z = -18.984375
              }
            },  
            {
              InstanceId = [[Client1_99]],  
              Class = [[Npc]],  
              Angle = -2.421875,  
              Base = [[palette.entities.botobjects.fo_s3_birch]],  
              InheritPos = 1,  
              Name = [[angelio II 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_97]],  
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
                InstanceId = [[Client1_100]],  
                Class = [[Position]],  
                x = 27090.85938,  
                y = -12950.73438,  
                z = -19.265625
              }
            },  
            {
              InstanceId = [[Client1_103]],  
              Class = [[Npc]],  
              Angle = -0.046875,  
              Base = [[palette.entities.botobjects.fo_s2_spiketree]],  
              InheritPos = 1,  
              Name = [[alinea 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_101]],  
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
                InstanceId = [[Client1_104]],  
                Class = [[Position]],  
                x = 27085.07813,  
                y = -12928.10938,  
                z = -18.90625
              }
            },  
            {
              InstanceId = [[Client1_174]],  
              Class = [[Region]],  
              InheritPos = 1,  
              Name = [[Mektoub Area 1]],  
              Points = {
                {
                  InstanceId = [[Client1_176]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_177]],  
                    Class = [[Position]],  
                    x = 27085.82813,  
                    y = -12931.25,  
                    z = -18.875
                  }
                },  
                {
                  InstanceId = [[Client1_179]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_180]],  
                    Class = [[Position]],  
                    x = 27103.5625,  
                    y = -12922.59375,  
                    z = -19.59375
                  }
                },  
                {
                  InstanceId = [[Client1_182]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_183]],  
                    Class = [[Position]],  
                    x = 27103.59375,  
                    y = -12942.48438,  
                    z = -19.296875
                  }
                },  
                {
                  InstanceId = [[Client1_185]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_186]],  
                    Class = [[Position]],  
                    x = 27089.60938,  
                    y = -12947.79688,  
                    z = -19.203125
                  }
                },  
                {
                  InstanceId = [[Client1_188]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_189]],  
                    Class = [[Position]],  
                    x = 27086.23438,  
                    y = -12940.82813,  
                    z = -19.078125
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_173]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_211]],  
              Class = [[Npc]],  
              Angle = -2.3125,  
              Base = [[palette.entities.botobjects.fo_s3_fougere]],  
              InheritPos = 1,  
              Name = [[arino 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_209]],  
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
                InstanceId = [[Client1_212]],  
                Class = [[Position]],  
                x = 27144.65625,  
                y = -12639.70313,  
                z = -9.046875
              }
            },  
            {
              InstanceId = [[Client1_215]],  
              Class = [[Npc]],  
              Angle = -2.3125,  
              Base = [[palette.entities.botobjects.fo_s3_fougere]],  
              InheritPos = 1,  
              Name = [[arino 3]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_213]],  
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
                InstanceId = [[Client1_216]],  
                Class = [[Position]],  
                x = 27169.54688,  
                y = -12630.23438,  
                z = -8.796875
              }
            },  
            {
              InstanceId = [[Client1_219]],  
              Class = [[Npc]],  
              Angle = -2.75,  
              Base = [[palette.entities.botobjects.fo_s2_birch]],  
              InheritPos = 1,  
              Name = [[angelio I 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_217]],  
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
                InstanceId = [[Client1_220]],  
                Class = [[Position]],  
                x = 27153.6875,  
                y = -12650.125,  
                z = -9.484375
              }
            },  
            {
              InstanceId = [[Client1_231]],  
              Class = [[Npc]],  
              Angle = 0.125,  
              Base = [[palette.entities.botobjects.house_ruin]],  
              InheritPos = 1,  
              Name = [[house ruin 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_229]],  
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
                InstanceId = [[Client1_232]],  
                Class = [[Position]],  
                x = 27132.45313,  
                y = -12646.98438,  
                z = -9.078125
              }
            },  
            {
              InstanceId = [[Client1_235]],  
              Class = [[Npc]],  
              Angle = -1.625,  
              Base = [[palette.entities.botobjects.ruin_wall]],  
              InheritPos = 1,  
              Name = [[wall ruin 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_233]],  
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
                InstanceId = [[Client1_236]],  
                Class = [[Position]],  
                x = 27160.125,  
                y = -12612.01563,  
                z = -9.921875
              }
            },  
            {
              InstanceId = [[Client1_239]],  
              Class = [[Npc]],  
              Angle = -2.6875,  
              Base = [[palette.entities.botobjects.ruin_wall_b]],  
              InheritPos = 1,  
              Name = [[wall ruin 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_237]],  
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
                InstanceId = [[Client1_240]],  
                Class = [[Position]],  
                x = 27175.75,  
                y = -12632.17188,  
                z = -8.75
              }
            },  
            {
              InstanceId = [[Client1_252]],  
              Class = [[Npc]],  
              Angle = 0.859375,  
              Base = [[palette.entities.botobjects.fx_fy_feu_foret]],  
              InheritPos = 1,  
              Name = [[fire 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_253]],  
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
                InstanceId = [[Client1_254]],  
                Class = [[Position]],  
                x = 27135.60938,  
                y = -12651.09375,  
                z = -9.359375
              }
            },  
            {
              InstanceId = [[Client1_262]],  
              Class = [[Npc]],  
              Angle = 0.859375,  
              Base = [[palette.entities.botobjects.fx_fy_feu_foret]],  
              InheritPos = 1,  
              Name = [[fire 3]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_263]],  
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
                InstanceId = [[Client1_264]],  
                Class = [[Position]],  
                x = 27135.01563,  
                y = -12643.10938,  
                z = -9.03125
              }
            },  
            {
              InstanceId = [[Client1_272]],  
              Class = [[Npc]],  
              Angle = 0.859375,  
              Base = [[palette.entities.botobjects.fx_fy_feu_foret]],  
              InheritPos = 1,  
              Name = [[fire 4]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_273]],  
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
                InstanceId = [[Client1_274]],  
                Class = [[Position]],  
                x = 27129.95313,  
                y = -12653.64063,  
                z = -9.609375
              }
            },  
            {
              InstanceId = [[Client1_282]],  
              Class = [[Npc]],  
              Angle = 0.859375,  
              Base = [[palette.entities.botobjects.fx_fy_feu_foret]],  
              InheritPos = 1,  
              Name = [[fire 5]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_283]],  
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
                InstanceId = [[Client1_284]],  
                Class = [[Position]],  
                x = 27127.54688,  
                y = -12645.375,  
                z = -9.171875
              }
            },  
            {
              InstanceId = [[Client1_292]],  
              Class = [[Npc]],  
              Angle = -2.71875,  
              Base = [[palette.entities.botobjects.fx_fy_feu_foret]],  
              InheritPos = 1,  
              Name = [[fire 6]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_290]],  
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
                InstanceId = [[Client1_293]],  
                Class = [[Position]],  
                x = 27174.875,  
                y = -12628.4375,  
                z = -8.625
              }
            },  
            {
              InstanceId = [[Client1_296]],  
              Class = [[Npc]],  
              Angle = 3.078125,  
              Base = [[palette.entities.botobjects.fx_fy_feu_foret]],  
              InheritPos = 1,  
              Name = [[fire 7]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_294]],  
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
                InstanceId = [[Client1_297]],  
                Class = [[Position]],  
                x = 27173.96875,  
                y = -12631.8125,  
                z = -8.796875
              }
            },  
            {
              InstanceId = [[Client1_304]],  
              Class = [[Npc]],  
              Angle = 3.09375,  
              Base = [[palette.entities.botobjects.fx_fy_feu_foret]],  
              InheritPos = 1,  
              Name = [[fire 9]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_302]],  
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
                InstanceId = [[Client1_305]],  
                Class = [[Position]],  
                x = 27175.90625,  
                y = -12635.70313,  
                z = -8.953125
              }
            },  
            {
              InstanceId = [[Client1_324]],  
              Class = [[Npc]],  
              Angle = -1.875,  
              Base = [[palette.entities.botobjects.fx_fy_feu_foret]],  
              InheritPos = 1,  
              Name = [[fire 10]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_322]],  
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
                InstanceId = [[Client1_325]],  
                Class = [[Position]],  
                x = 27157.10938,  
                y = -12611.48438,  
                z = -9.953125
              }
            },  
            {
              InstanceId = [[Client1_328]],  
              Class = [[Npc]],  
              Angle = -1.875,  
              Base = [[palette.entities.botobjects.fx_fy_feu_foret]],  
              InheritPos = 1,  
              Name = [[fire 11]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_326]],  
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
                InstanceId = [[Client1_329]],  
                Class = [[Position]],  
                x = 27159.57813,  
                y = -12614.90625,  
                z = -9.6875
              }
            },  
            {
              InstanceId = [[Client1_336]],  
              Class = [[Npc]],  
              Angle = -1.46875,  
              Base = [[palette.entities.botobjects.barrel_broken_1]],  
              InheritPos = 1,  
              Name = [[broken barrels 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_334]],  
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
                InstanceId = [[Client1_337]],  
                Class = [[Position]],  
                x = 27148.82813,  
                y = -12630.60938,  
                z = -8.921875
              }
            },  
            {
              InstanceId = [[Client1_340]],  
              Class = [[Npc]],  
              Angle = -0.734375,  
              Base = [[palette.entities.botobjects.campfire_out]],  
              InheritPos = 1,  
              Name = [[dead camp fire 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_338]],  
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
                InstanceId = [[Client1_341]],  
                Class = [[Position]],  
                x = 27153.53125,  
                y = -12632.15625,  
                z = -8.953125
              }
            },  
            {
              InstanceId = [[Client1_344]],  
              Class = [[Npc]],  
              Angle = -2.25,  
              Base = [[palette.entities.botobjects.jar]],  
              InheritPos = 1,  
              Name = [[jar 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_342]],  
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
                InstanceId = [[Client1_345]],  
                Class = [[Position]],  
                x = 27168.71875,  
                y = -12630.96875,  
                z = -8.8125
              }
            },  
            {
              InstanceId = [[Client1_368]],  
              Class = [[Npc]],  
              Angle = 2.828125,  
              Base = [[palette.entities.botobjects.bag_a]],  
              InheritPos = 1,  
              Name = [[bag 3]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_366]],  
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
                InstanceId = [[Client1_369]],  
                Class = [[Position]],  
                x = 27164.71875,  
                y = -12633.09375,  
                z = -8.921875
              }
            },  
            {
              InstanceId = [[Client1_384]],  
              Class = [[Npc]],  
              Angle = -1.453125,  
              Base = [[palette.entities.botobjects.fx_fo_solbirthb]],  
              InheritPos = 1,  
              Name = [[blown leaves 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_382]],  
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
                InstanceId = [[Client1_385]],  
                Class = [[Position]],  
                x = 27148.375,  
                y = -12647.53125,  
                z = -9.140625
              }
            },  
            {
              InstanceId = [[Client1_388]],  
              Class = [[Npc]],  
              Angle = -0.546875,  
              Base = [[palette.entities.botobjects.spot_kitin]],  
              InheritPos = 1,  
              Name = [[kitin mound 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_386]],  
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
                InstanceId = [[Client1_389]],  
                Class = [[Position]],  
                x = 27069.76563,  
                y = -12668.54688,  
                z = -13.015625
              }
            },  
            {
              InstanceId = [[Client1_438]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Route 1]],  
              Points = {
                {
                  InstanceId = [[Client1_440]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_441]],  
                    Class = [[Position]],  
                    x = 27101.28125,  
                    y = -12721.625,  
                    z = -18
                  }
                },  
                {
                  InstanceId = [[Client1_443]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_444]],  
                    Class = [[Position]],  
                    x = 27004.85938,  
                    y = -12761.85938,  
                    z = -22.765625
                  }
                },  
                {
                  InstanceId = [[Client1_446]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_447]],  
                    Class = [[Position]],  
                    x = 27068.54688,  
                    y = -12710.03125,  
                    z = -18.8125
                  }
                },  
                {
                  InstanceId = [[Client1_449]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_450]],  
                    Class = [[Position]],  
                    x = 27100.3125,  
                    y = -12719.89063,  
                    z = -18.9375
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_437]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_513]],  
              Class = [[Region]],  
              InheritPos = 1,  
              Name = [[Kitin Farm]],  
              Points = {
                {
                  InstanceId = [[Client1_515]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_516]],  
                    Class = [[Position]],  
                    x = 27133.10938,  
                    y = -12609.34375,  
                    z = -13.125
                  }
                },  
                {
                  InstanceId = [[Client1_518]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_519]],  
                    Class = [[Position]],  
                    x = 27164.85938,  
                    y = -12587.5625,  
                    z = -14.25
                  }
                },  
                {
                  InstanceId = [[Client1_521]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_522]],  
                    Class = [[Position]],  
                    x = 27188.5,  
                    y = -12630.65625,  
                    z = -9.1875
                  }
                },  
                {
                  InstanceId = [[Client1_524]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_525]],  
                    Class = [[Position]],  
                    x = 27178.54688,  
                    y = -12667.90625,  
                    z = -14.046875
                  }
                },  
                {
                  InstanceId = [[Client1_527]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_528]],  
                    Class = [[Position]],  
                    x = 27147.46875,  
                    y = -12666,  
                    z = -11.625
                  }
                },  
                {
                  InstanceId = [[Client1_530]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_531]],  
                    Class = [[Position]],  
                    x = 27139.375,  
                    y = -12640.73438,  
                    z = -8.984375
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_512]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_712]],  
              Class = [[Npc]],  
              Angle = -2.15625,  
              Base = [[palette.entities.botobjects.fo_s3_birch]],  
              InheritPos = 1,  
              Name = [[angelio II 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_710]],  
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
                InstanceId = [[Client1_713]],  
                Class = [[Position]],  
                x = 27102.70313,  
                y = -12682.76563,  
                z = -15.390625
              }
            },  
            {
              InstanceId = [[Client1_732]],  
              Class = [[Npc]],  
              Angle = 0.265625,  
              Base = [[palette.entities.botobjects.fo_s3_fougere]],  
              InheritPos = 1,  
              Name = [[arino 7]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_730]],  
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
                InstanceId = [[Client1_733]],  
                Class = [[Position]],  
                x = 26994.07813,  
                y = -12670.21875,  
                z = -20.0625
              }
            },  
            {
              InstanceId = [[Client1_736]],  
              Class = [[Npc]],  
              Angle = 0.578125,  
              Base = [[palette.entities.botobjects.fo_s2_spiketree]],  
              InheritPos = 1,  
              Name = [[alinea 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_734]],  
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
                InstanceId = [[Client1_737]],  
                Class = [[Position]],  
                x = 27007.1875,  
                y = -12692.90625,  
                z = -18.359375
              }
            },  
            {
              InstanceId = [[Client1_848]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Route 2]],  
              Points = {
                {
                  InstanceId = [[Client1_850]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_851]],  
                    Class = [[Position]],  
                    x = 27082.14063,  
                    y = -12943.39063,  
                    z = -19.234375
                  }
                },  
                {
                  InstanceId = [[Client1_853]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_854]],  
                    Class = [[Position]],  
                    x = 27029.875,  
                    y = -12939.875,  
                    z = -19.265625
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_847]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_864]],  
              Class = [[Region]],  
              InheritPos = 1,  
              Name = [[Place 1]],  
              Points = {
                {
                  InstanceId = [[Client1_866]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_867]],  
                    Class = [[Position]],  
                    x = 27047.32813,  
                    y = -12944.73438,  
                    z = -18.671875
                  }
                },  
                {
                  InstanceId = [[Client1_869]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_870]],  
                    Class = [[Position]],  
                    x = 27056.15625,  
                    y = -12938.84375,  
                    z = -18.71875
                  }
                },  
                {
                  InstanceId = [[Client1_872]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_873]],  
                    Class = [[Position]],  
                    x = 27043.625,  
                    y = -12937.57813,  
                    z = -18.515625
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_863]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_896]],  
              Class = [[Npc]],  
              Angle = 2.65625,  
              Base = [[palette.entities.botobjects.watch_tower]],  
              InheritPos = 1,  
              Name = [[watch tower 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_894]],  
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
                InstanceId = [[Client1_897]],  
                Class = [[Position]],  
                x = 32705.25,  
                y = -21089.5625,  
                z = 8.703125
              }
            },  
            {
              InstanceId = [[Client1_900]],  
              Class = [[Npc]],  
              Angle = 2.90625,  
              Base = [[palette.entities.botobjects.pack_3]],  
              InheritPos = 1,  
              Name = [[pack 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_898]],  
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
                InstanceId = [[Client1_901]],  
                Class = [[Position]],  
                x = 32702.6875,  
                y = -21082.70313,  
                z = 8.375
              }
            },  
            {
              InstanceId = [[Client1_904]],  
              Class = [[Npc]],  
              Angle = 2.5,  
              Base = [[palette.entities.botobjects.bag_b]],  
              InheritPos = 1,  
              Name = [[bag 5]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_902]],  
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
                InstanceId = [[Client1_905]],  
                Class = [[Position]],  
                x = 32691.98438,  
                y = -21089.01563,  
                z = 8.125
              }
            },  
            {
              InstanceId = [[Client1_908]],  
              Class = [[Npc]],  
              Angle = -2.421875,  
              Base = [[palette.entities.botobjects.campfire]],  
              InheritPos = 1,  
              Name = [[camp fire 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_906]],  
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
                InstanceId = [[Client1_909]],  
                Class = [[Position]],  
                x = 32692.42188,  
                y = -21076.9375,  
                z = 7.296875
              }
            },  
            {
              InstanceId = [[Client1_1058]],  
              Class = [[Npc]],  
              Angle = 2.96875,  
              Base = [[palette.entities.botobjects.watch_tower]],  
              InheritPos = 1,  
              Name = [[watch tower 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1056]],  
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
                InstanceId = [[Client1_1059]],  
                Class = [[Position]],  
                x = 32302.53125,  
                y = -21470.90625,  
                z = -21.15625
              }
            },  
            {
              InstanceId = [[Client1_1062]],  
              Class = [[Npc]],  
              Angle = -1.671875,  
              Base = [[palette.entities.botobjects.watch_tower]],  
              InheritPos = 1,  
              Name = [[watch tower 3]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1060]],  
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
                InstanceId = [[Client1_1063]],  
                Class = [[Position]],  
                x = 32265.21875,  
                y = -21447.1875,  
                z = -21.453125
              }
            },  
            {
              InstanceId = [[Client1_1070]],  
              Class = [[Npc]],  
              Angle = 1.75,  
              Base = [[palette.entities.botobjects.paddock]],  
              InheritPos = 1,  
              Name = [[paddock 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1068]],  
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
                InstanceId = [[Client1_1071]],  
                Class = [[Position]],  
                x = 32276.14063,  
                y = -21492.5625,  
                z = -23.765625
              }
            },  
            {
              InstanceId = [[Client1_1082]],  
              Class = [[Npc]],  
              Angle = -2.09375,  
              Base = [[palette.entities.botobjects.hut]],  
              InheritPos = 1,  
              Name = [[hut 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1080]],  
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
                InstanceId = [[Client1_1083]],  
                Class = [[Position]],  
                x = 32282.85938,  
                y = -21457.95313,  
                z = -21.53125
              }
            },  
            {
              InstanceId = [[Client1_1086]],  
              Class = [[Npc]],  
              Angle = 1.0625,  
              Base = [[palette.entities.botobjects.tent_tryker]],  
              InheritPos = 1,  
              Name = [[tryker tent 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1084]],  
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
                InstanceId = [[Client1_1087]],  
                Class = [[Position]],  
                x = 32252.85938,  
                y = -21485.84375,  
                z = -21.828125
              }
            },  
            {
              InstanceId = [[Client1_1090]],  
              Class = [[Npc]],  
              Angle = 0.28125,  
              Base = [[palette.entities.botobjects.tent_tryker]],  
              InheritPos = 1,  
              Name = [[tryker tent 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1088]],  
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
                InstanceId = [[Client1_1091]],  
                Class = [[Position]],  
                x = 32247.125,  
                y = -21479.90625,  
                z = -21.4375
              }
            },  
            {
              InstanceId = [[Client1_1094]],  
              Class = [[Npc]],  
              Angle = -0.859375,  
              Base = [[palette.entities.botobjects.tent_tryker]],  
              InheritPos = 1,  
              Name = [[tryker tent 3]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1092]],  
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
                InstanceId = [[Client1_1095]],  
                Class = [[Position]],  
                x = 32251.07813,  
                y = -21470.9375,  
                z = -20.34375
              }
            },  
            {
              InstanceId = [[Client1_1098]],  
              Class = [[Npc]],  
              Angle = 2.921875,  
              Base = [[palette.entities.botobjects.tent_tryker]],  
              InheritPos = 1,  
              Name = [[tryker tent 4]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1096]],  
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
                InstanceId = [[Client1_1099]],  
                Class = [[Position]],  
                x = 32296.15625,  
                y = -21480.70313,  
                z = -21.34375
              }
            },  
            {
              InstanceId = [[Client1_1106]],  
              Class = [[Npc]],  
              Angle = -0.796875,  
              Base = [[palette.entities.botobjects.campfire]],  
              InheritPos = 1,  
              Name = [[camp fire 3]],  
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
                },  
                ChatSequences = {
                }
              },  
              Position = {
                InstanceId = [[Client1_1107]],  
                Class = [[Position]],  
                x = 32255.92188,  
                y = -21478.6875,  
                z = -20.765625
              }
            },  
            {
              InstanceId = [[Client1_1110]],  
              Class = [[Npc]],  
              Angle = 1.84375,  
              Base = [[palette.entities.botobjects.street_lamp]],  
              InheritPos = 1,  
              Name = [[street lamp 2]],  
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
                x = 32265.53125,  
                y = -21489.09375,  
                z = -22.78125
              }
            },  
            {
              InstanceId = [[Client1_1114]],  
              Class = [[Npc]],  
              Angle = -1.3125,  
              Base = [[palette.entities.botobjects.street_lamp]],  
              InheritPos = 1,  
              Name = [[street lamp 3]],  
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
              Position = {
                InstanceId = [[Client1_1115]],  
                Class = [[Position]],  
                x = 32253.90625,  
                y = -21459.04688,  
                z = -20.4375
              }
            },  
            {
              InstanceId = [[Client1_1118]],  
              Class = [[Npc]],  
              Angle = -2.46875,  
              Base = [[palette.entities.botobjects.street_lamp]],  
              InheritPos = 1,  
              Name = [[street lamp 4]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1116]],  
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
                x = 32292.15625,  
                y = -21462.20313,  
                z = -20.875
              }
            },  
            {
              InstanceId = [[Client1_1122]],  
              Class = [[Npc]],  
              Angle = 2.484375,  
              Base = [[palette.entities.botobjects.street_lamp]],  
              InheritPos = 1,  
              Name = [[street lamp 5]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1120]],  
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
                InstanceId = [[Client1_1123]],  
                Class = [[Position]],  
                x = 32292.54688,  
                y = -21490.51563,  
                z = -22.6875
              }
            },  
            {
              InstanceId = [[Client1_1126]],  
              Class = [[Npc]],  
              Angle = 1.03125,  
              Base = [[palette.entities.botobjects.fo_s3_birch]],  
              InheritPos = 1,  
              Name = [[angelio II 3]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1124]],  
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
                InstanceId = [[Client1_1127]],  
                Class = [[Position]],  
                x = 27046.07813,  
                y = -12795.59375,  
                z = -19.921875
              }
            },  
            {
              InstanceId = [[Client1_1130]],  
              Class = [[Npc]],  
              Angle = -0.328125,  
              Base = [[palette.entities.botobjects.pack_2]],  
              InheritPos = 1,  
              Name = [[pack 3]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1128]],  
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
                InstanceId = [[Client1_1131]],  
                Class = [[Position]],  
                x = 32256.70313,  
                y = -21467.59375,  
                z = -19.984375
              }
            },  
            {
              InstanceId = [[Client1_1134]],  
              Class = [[Npc]],  
              Angle = 2.5,  
              Base = [[palette.entities.botobjects.pack_4]],  
              InheritPos = 1,  
              Name = [[pack 4]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1132]],  
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
                InstanceId = [[Client1_1135]],  
                Class = [[Position]],  
                x = 32282.14063,  
                y = -21488.5625,  
                z = -23.203125
              }
            },  
            {
              InstanceId = [[Client1_1138]],  
              Class = [[Npc]],  
              Angle = -2.28125,  
              Base = [[palette.entities.botobjects.pack_2]],  
              InheritPos = 1,  
              Name = [[pack 5]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1136]],  
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
                InstanceId = [[Client1_1139]],  
                Class = [[Position]],  
                x = 32294.89063,  
                y = -21475.54688,  
                z = -20.96875
              }
            },  
            {
              InstanceId = [[Client1_1201]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Route 3]],  
              Points = {
                {
                  InstanceId = [[Client1_1203]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1204]],  
                    Class = [[Position]],  
                    x = 32687.17188,  
                    y = -21091.0625,  
                    z = 7.515625
                  }
                },  
                {
                  InstanceId = [[Client1_1206]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1207]],  
                    Class = [[Position]],  
                    x = 32597.14063,  
                    y = -21191.48438,  
                    z = 7.046875
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_1200]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_1407]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Route 4]],  
              Points = {
                {
                  InstanceId = [[Client1_1409]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1410]],  
                    Class = [[Position]],  
                    x = 32579.09375,  
                    y = -21197.45313,  
                    z = 6.09375
                  }
                },  
                {
                  InstanceId = [[Client1_1412]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1413]],  
                    Class = [[Position]],  
                    x = 32526.28125,  
                    y = -21204.65625,  
                    z = -18.671875
                  }
                },  
                {
                  InstanceId = [[Client1_1415]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1416]],  
                    Class = [[Position]],  
                    x = 32474.96875,  
                    y = -21221.79688,  
                    z = -23.1875
                  }
                },  
                {
                  InstanceId = [[Client1_1418]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1419]],  
                    Class = [[Position]],  
                    x = 32372.39063,  
                    y = -21223.25,  
                    z = -15.671875
                  }
                },  
                {
                  InstanceId = [[Client1_1421]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1422]],  
                    Class = [[Position]],  
                    x = 32338.48438,  
                    y = -21227.71875,  
                    z = -5.890625
                  }
                },  
                {
                  InstanceId = [[Client1_1424]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1425]],  
                    Class = [[Position]],  
                    x = 32326.4375,  
                    y = -21281.70313,  
                    z = -0.3125
                  }
                },  
                {
                  InstanceId = [[Client1_1427]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1428]],  
                    Class = [[Position]],  
                    x = 32318.9375,  
                    y = -21363.8125,  
                    z = -17.578125
                  }
                },  
                {
                  InstanceId = [[Client1_1430]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1431]],  
                    Class = [[Position]],  
                    x = 32281.09375,  
                    y = -21439.01563,  
                    z = -21.71875
                  }
                },  
                {
                  InstanceId = [[Client1_1433]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1434]],  
                    Class = [[Position]],  
                    x = 32261.1875,  
                    y = -21484.4375,  
                    z = -21.703125
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_1406]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_1680]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Route 5]],  
              Points = {
                {
                  InstanceId = [[Client1_1682]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1683]],  
                    Class = [[Position]],  
                    x = 32688.89063,  
                    y = -21091.21875,  
                    z = 7.703125
                  }
                },  
                {
                  InstanceId = [[Client1_1685]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1686]],  
                    Class = [[Position]],  
                    x = 32668.96875,  
                    y = -21148.01563,  
                    z = 6.671875
                  }
                },  
                {
                  InstanceId = [[Client1_1688]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1689]],  
                    Class = [[Position]],  
                    x = 32649.17188,  
                    y = -21214,  
                    z = 0.78125
                  }
                },  
                {
                  InstanceId = [[Client1_1691]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1692]],  
                    Class = [[Position]],  
                    x = 32633.75,  
                    y = -21282.25,  
                    z = 2.390625
                  }
                },  
                {
                  InstanceId = [[Client1_1694]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1695]],  
                    Class = [[Position]],  
                    x = 32632.35938,  
                    y = -21332.07813,  
                    z = 3.296875
                  }
                },  
                {
                  InstanceId = [[Client1_1697]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1698]],  
                    Class = [[Position]],  
                    x = 32618.4375,  
                    y = -21390.76563,  
                    z = -1.8125
                  }
                },  
                {
                  InstanceId = [[Client1_1700]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1701]],  
                    Class = [[Position]],  
                    x = 32616.85938,  
                    y = -21415.70313,  
                    z = -4.0625
                  }
                },  
                {
                  InstanceId = [[Client1_1703]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1704]],  
                    Class = [[Position]],  
                    x = 32575.84375,  
                    y = -21447.59375,  
                    z = -21.078125
                  }
                },  
                {
                  InstanceId = [[Client1_1706]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1707]],  
                    Class = [[Position]],  
                    x = 32542.6875,  
                    y = -21485.59375,  
                    z = -19.828125
                  }
                },  
                {
                  InstanceId = [[Client1_1709]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1710]],  
                    Class = [[Position]],  
                    x = 32470.5,  
                    y = -21546.26563,  
                    z = -21.625
                  }
                },  
                {
                  InstanceId = [[Client1_1712]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1713]],  
                    Class = [[Position]],  
                    x = 32418.75,  
                    y = -21544.10938,  
                    z = -19.546875
                  }
                },  
                {
                  InstanceId = [[Client1_1715]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1716]],  
                    Class = [[Position]],  
                    x = 32354.54688,  
                    y = -21521.23438,  
                    z = -21.875
                  }
                },  
                {
                  InstanceId = [[Client1_1718]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1719]],  
                    Class = [[Position]],  
                    x = 32262.25,  
                    y = -21485.42188,  
                    z = -21
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_1679]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_1787]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Route 6]],  
              Points = {
                {
                  InstanceId = [[Client1_1789]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1790]],  
                    Class = [[Position]],  
                    x = 32653.85938,  
                    y = -21207.98438,  
                    z = 0.453125
                  }
                },  
                {
                  InstanceId = [[Client1_1792]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1793]],  
                    Class = [[Position]],  
                    x = 32600.79688,  
                    y = -21183.15625,  
                    z = 6.453125
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_1786]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_1816]],  
              Class = [[Region]],  
              InheritPos = 1,  
              Name = [[Place 2]],  
              Points = {
                {
                  InstanceId = [[Client1_1818]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1819]],  
                    Class = [[Position]],  
                    x = 32690.82813,  
                    y = -21283.29688,  
                    z = 2.625
                  }
                },  
                {
                  InstanceId = [[Client1_1821]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1822]],  
                    Class = [[Position]],  
                    x = 32703.45313,  
                    y = -21338.28125,  
                    z = 4.390625
                  }
                },  
                {
                  InstanceId = [[Client1_1824]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1825]],  
                    Class = [[Position]],  
                    x = 32620.60938,  
                    y = -21334.51563,  
                    z = 5.90625
                  }
                },  
                {
                  InstanceId = [[Client1_1827]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1828]],  
                    Class = [[Position]],  
                    x = 32622.04688,  
                    y = -21265.59375,  
                    z = 4.71875
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_1815]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_1860]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Route 7]],  
              Points = {
                {
                  InstanceId = [[Client1_1862]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1863]],  
                    Class = [[Position]],  
                    x = 32605.20313,  
                    y = -21449.07813,  
                    z = -10.796875
                  }
                },  
                {
                  InstanceId = [[Client1_1865]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1866]],  
                    Class = [[Position]],  
                    x = 32590.32813,  
                    y = -21417.59375,  
                    z = -15.015625
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_1859]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_1927]],  
              Class = [[Region]],  
              InheritPos = 1,  
              Name = [[Place 3]],  
              Points = {
                {
                  InstanceId = [[Client1_1929]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1930]],  
                    Class = [[Position]],  
                    x = 32553.70313,  
                    y = -21481.51563,  
                    z = -19.265625
                  }
                },  
                {
                  InstanceId = [[Client1_1932]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1933]],  
                    Class = [[Position]],  
                    x = 32512.89063,  
                    y = -21529.92188,  
                    z = -20.40625
                  }
                },  
                {
                  InstanceId = [[Client1_1935]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1936]],  
                    Class = [[Position]],  
                    x = 32472.39063,  
                    y = -21464.78125,  
                    z = -20.640625
                  }
                },  
                {
                  InstanceId = [[Client1_1938]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1939]],  
                    Class = [[Position]],  
                    x = 32528.42188,  
                    y = -21417.625,  
                    z = -21.625
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_1926]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_1960]],  
              Class = [[Region]],  
              InheritPos = 1,  
              Name = [[Place 4]],  
              Points = {
                {
                  InstanceId = [[Client1_1962]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1963]],  
                    Class = [[Position]],  
                    x = 32377.70313,  
                    y = -21518.21875,  
                    z = -24.25
                  }
                },  
                {
                  InstanceId = [[Client1_1965]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1966]],  
                    Class = [[Position]],  
                    x = 32378.89063,  
                    y = -21532.40625,  
                    z = -22.6875
                  }
                },  
                {
                  InstanceId = [[Client1_1968]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1969]],  
                    Class = [[Position]],  
                    x = 32399.4375,  
                    y = -21541.04688,  
                    z = -19.8125
                  }
                },  
                {
                  InstanceId = [[Client1_1971]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1972]],  
                    Class = [[Position]],  
                    x = 32411.40625,  
                    y = -21528.0625,  
                    z = -22.34375
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_1959]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_1980]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Route 8]],  
              Points = {
                {
                  InstanceId = [[Client1_1982]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1983]],  
                    Class = [[Position]],  
                    x = 32530.70313,  
                    y = -21195.25,  
                    z = -15.046875
                  }
                },  
                {
                  InstanceId = [[Client1_1985]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_1986]],  
                    Class = [[Position]],  
                    x = 32529.21875,  
                    y = -21224.10938,  
                    z = -18.375
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_1979]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_2007]],  
              Class = [[Region]],  
              InheritPos = 1,  
              Name = [[Place 5]],  
              Points = {
                {
                  InstanceId = [[Client1_2009]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_2010]],  
                    Class = [[Position]],  
                    x = 32469.46875,  
                    y = -21230.125,  
                    z = -22.78125
                  }
                },  
                {
                  InstanceId = [[Client1_2012]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_2013]],  
                    Class = [[Position]],  
                    x = 32481.28125,  
                    y = -21130.76563,  
                    z = -15.1875
                  }
                },  
                {
                  InstanceId = [[Client1_2015]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_2016]],  
                    Class = [[Position]],  
                    x = 32349.84375,  
                    y = -21153.10938,  
                    z = -18.390625
                  }
                },  
                {
                  InstanceId = [[Client1_2018]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_2019]],  
                    Class = [[Position]],  
                    x = 32332.82813,  
                    y = -21185.78125,  
                    z = -10.65625
                  }
                },  
                {
                  InstanceId = [[Client1_2021]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_2022]],  
                    Class = [[Position]],  
                    x = 32326.6875,  
                    y = -21234.875,  
                    z = -1.78125
                  }
                },  
                {
                  InstanceId = [[Client1_2024]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_2025]],  
                    Class = [[Position]],  
                    x = 32401.48438,  
                    y = -21229,  
                    z = -18.671875
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_2006]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_2130]],  
              Class = [[Region]],  
              InheritPos = 1,  
              Name = [[Place 6]],  
              Points = {
                {
                  InstanceId = [[Client1_2132]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_2133]],  
                    Class = [[Position]],  
                    x = 32337.96875,  
                    y = -21294.23438,  
                    z = 1.515625
                  }
                },  
                {
                  InstanceId = [[Client1_2135]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_2136]],  
                    Class = [[Position]],  
                    x = 32302.25,  
                    y = -21293.35938,  
                    z = 1.84375
                  }
                },  
                {
                  InstanceId = [[Client1_2138]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_2139]],  
                    Class = [[Position]],  
                    x = 32293.625,  
                    y = -21319.15625,  
                    z = 2.921875
                  }
                },  
                {
                  InstanceId = [[Client1_2141]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_2142]],  
                    Class = [[Position]],  
                    x = 32344.42188,  
                    y = -21341.45313,  
                    z = -8.890625
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_2129]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_2785]],  
              Class = [[Npc]],  
              Angle = -1.328125,  
              Base = [[palette.entities.botobjects.campfire]],  
              InheritPos = 1,  
              Name = [[camp fire 4]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2783]],  
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
                InstanceId = [[Client1_2786]],  
                Class = [[Position]],  
                x = 36446.625,  
                y = -1211.859375,  
                z = -19.4375
              }
            },  
            {
              InstanceId = [[Client1_2789]],  
              Class = [[Npc]],  
              Angle = 2.53125,  
              Base = [[palette.entities.botobjects.tent_fyros]],  
              InheritPos = 1,  
              Name = [[fyros tent 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2787]],  
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
                InstanceId = [[Client1_2790]],  
                Class = [[Position]],  
                x = 36457.125,  
                y = -1218.78125,  
                z = -19.921875
              }
            },  
            {
              InstanceId = [[Client1_2793]],  
              Class = [[Npc]],  
              Angle = 2.109375,  
              Base = [[palette.entities.botobjects.tent_fyros]],  
              InheritPos = 1,  
              Name = [[fyros tent 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2791]],  
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
                InstanceId = [[Client1_2794]],  
                Class = [[Position]],  
                x = 36451.60938,  
                y = -1223.34375,  
                z = -20.421875
              }
            },  
            {
              InstanceId = [[Client1_2797]],  
              Class = [[Npc]],  
              Angle = 4.21875,  
              Base = [[palette.entities.botobjects.tent_fyros]],  
              InheritPos = 1,  
              Name = [[fyros tent 3]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2795]],  
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
                InstanceId = [[Client1_2798]],  
                Class = [[Position]],  
                x = 36453.73438,  
                y = -1199.515625,  
                z = -17.859375
              }
            },  
            {
              InstanceId = [[Client1_2801]],  
              Class = [[Npc]],  
              Angle = 0.28125,  
              Base = [[palette.entities.botobjects.tent_fyros]],  
              InheritPos = 1,  
              Name = [[fyros tent 4]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2799]],  
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
                InstanceId = [[Client1_2802]],  
                Class = [[Position]],  
                x = 36435.34375,  
                y = -1220.546875,  
                z = -18.203125
              }
            },  
            {
              InstanceId = [[Client1_2805]],  
              Class = [[Npc]],  
              Angle = 0.28125,  
              Base = [[palette.entities.botobjects.tent_fyros]],  
              InheritPos = 1,  
              Name = [[fyros tent 5]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2803]],  
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
                InstanceId = [[Client1_2806]],  
                Class = [[Position]],  
                x = 36433.26563,  
                y = -1211.859375,  
                z = -17.3125
              }
            },  
            {
              InstanceId = [[Client1_2809]],  
              Class = [[Npc]],  
              Angle = -0.375,  
              Base = [[palette.entities.botobjects.barrier]],  
              InheritPos = 1,  
              Name = [[barrier 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2807]],  
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
                InstanceId = [[Client1_2810]],  
                Class = [[Position]],  
                x = 36409.29688,  
                y = -1210.8125,  
                z = -15.375
              }
            },  
            {
              InstanceId = [[Client1_2813]],  
              Class = [[Npc]],  
              Angle = -0.4375,  
              Base = [[palette.entities.botobjects.barrier]],  
              InheritPos = 1,  
              Name = [[barrier 2]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2811]],  
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
                InstanceId = [[Client1_2814]],  
                Class = [[Position]],  
                x = 36410.89063,  
                y = -1207.796875,  
                z = -14.8125
              }
            },  
            {
              InstanceId = [[Client1_2817]],  
              Class = [[Npc]],  
              Angle = -0.46875,  
              Base = [[palette.entities.botobjects.barrier]],  
              InheritPos = 1,  
              Name = [[barrier 3]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2815]],  
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
                InstanceId = [[Client1_2818]],  
                Class = [[Position]],  
                x = 36412.35938,  
                y = -1204.65625,  
                z = -14.53125
              }
            },  
            {
              InstanceId = [[Client1_2821]],  
              Class = [[Npc]],  
              Angle = -0.46875,  
              Base = [[palette.entities.botobjects.barrier]],  
              InheritPos = 1,  
              Name = [[barrier 4]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2819]],  
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
                InstanceId = [[Client1_2822]],  
                Class = [[Position]],  
                x = 36414.17188,  
                y = -1201.78125,  
                z = -13.953125
              }
            },  
            {
              InstanceId = [[Client1_2825]],  
              Class = [[Npc]],  
              Angle = -0.4375,  
              Base = [[palette.entities.botobjects.barrier]],  
              InheritPos = 1,  
              Name = [[barrier 5]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2823]],  
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
                InstanceId = [[Client1_2826]],  
                Class = [[Position]],  
                x = 36415.71875,  
                y = -1198.8125,  
                z = -13.796875
              }
            },  
            {
              InstanceId = [[Client1_2829]],  
              Class = [[Npc]],  
              Angle = -0.4375,  
              Base = [[palette.entities.botobjects.barrier]],  
              InheritPos = 1,  
              Name = [[barrier 6]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2827]],  
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
                InstanceId = [[Client1_2830]],  
                Class = [[Position]],  
                x = 36417.09375,  
                y = -1195.609375,  
                z = -14.125
              }
            },  
            {
              InstanceId = [[Client1_2833]],  
              Class = [[Npc]],  
              Angle = -0.53125,  
              Base = [[palette.entities.botobjects.barrier]],  
              InheritPos = 1,  
              Name = [[barrier 7]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2831]],  
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
                InstanceId = [[Client1_2834]],  
                Class = [[Position]],  
                x = 36418.3125,  
                y = -1192.375,  
                z = -14.53125
              }
            },  
            {
              InstanceId = [[Client1_2837]],  
              Class = [[Npc]],  
              Angle = -0.53125,  
              Base = [[palette.entities.botobjects.barrier]],  
              InheritPos = 1,  
              Name = [[barrier 8]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2835]],  
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
                InstanceId = [[Client1_2838]],  
                Class = [[Position]],  
                x = 36419.65625,  
                y = -1189.25,  
                z = -15.1875
              }
            },  
            {
              InstanceId = [[Client1_2841]],  
              Class = [[Npc]],  
              Angle = -0.375,  
              Base = [[palette.entities.botobjects.barrier]],  
              InheritPos = 1,  
              Name = [[barrier 9]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2839]],  
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
                InstanceId = [[Client1_2842]],  
                Class = [[Position]],  
                x = 36420.375,  
                y = -1187.265625,  
                z = -15.09375
              }
            },  
            {
              InstanceId = [[Client1_2848]],  
              Class = [[Npc]],  
              Angle = -0.234375,  
              Base = [[palette.entities.botobjects.watch_tower]],  
              InheritPos = 1,  
              Name = [[watch tower 4]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2846]],  
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
                InstanceId = [[Client1_2849]],  
                Class = [[Position]],  
                x = 36422.10938,  
                y = -1201.859375,  
                z = -14.28125
              }
            },  
            {
              InstanceId = [[Client1_2852]],  
              Class = [[Npc]],  
              Angle = -2.21875,  
              Base = [[palette.entities.botobjects.pack_4]],  
              InheritPos = 1,  
              Name = [[pack 6]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2850]],  
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
                InstanceId = [[Client1_2853]],  
                Class = [[Position]],  
                x = 36442.40625,  
                y = -1196.484375,  
                z = -16.3125
              }
            },  
            {
              InstanceId = [[Client1_2856]],  
              Class = [[Npc]],  
              Angle = -1.40625,  
              Base = [[palette.entities.botobjects.pack_5]],  
              InheritPos = 1,  
              Name = [[pack 7]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2854]],  
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
                InstanceId = [[Client1_2857]],  
                Class = [[Position]],  
                x = 36438.70313,  
                y = -1196,  
                z = -15.796875
              }
            },  
            {
              InstanceId = [[Client1_2860]],  
              Class = [[Npc]],  
              Angle = 1.8125,  
              Base = [[palette.entities.botobjects.pack_1]],  
              InheritPos = 1,  
              Name = [[pack 8]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2858]],  
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
                InstanceId = [[Client1_2861]],  
                Class = [[Position]],  
                x = 36445,  
                y = -1229.3125,  
                z = -21.828125
              }
            },  
            {
              InstanceId = [[Client1_2864]],  
              Class = [[Npc]],  
              Angle = 2.171875,  
              Base = [[palette.entities.botobjects.bag_a]],  
              InheritPos = 1,  
              Name = [[bag 6]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2862]],  
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
                InstanceId = [[Client1_2865]],  
                Class = [[Position]],  
                x = 36449.8125,  
                y = -1206.390625,  
                z = -18.75
              }
            },  
            {
              InstanceId = [[Client1_2868]],  
              Class = [[Npc]],  
              Angle = 2.171875,  
              Base = [[palette.entities.botobjects.bag_a]],  
              InheritPos = 1,  
              Name = [[bag 7]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2866]],  
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
                InstanceId = [[Client1_2869]],  
                Class = [[Position]],  
                x = 36438.10938,  
                y = -1207.609375,  
                z = -17.828125
              }
            },  
            {
              InstanceId = [[Client1_2871]],  
              Class = [[Region]],  
              InheritPos = 1,  
              Name = [[Place 7]],  
              Points = {
                {
                  InstanceId = [[Client1_2873]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_2874]],  
                    Class = [[Position]],  
                    x = 36434.03125,  
                    y = -1198.859375,  
                    z = -15.671875
                  }
                },  
                {
                  InstanceId = [[Client1_2876]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_2877]],  
                    Class = [[Position]],  
                    x = 36469.82813,  
                    y = -1191.75,  
                    z = -19.921875
                  }
                },  
                {
                  InstanceId = [[Client1_2879]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_2880]],  
                    Class = [[Position]],  
                    x = 36467.39063,  
                    y = -1233.375,  
                    z = -14.390625
                  }
                },  
                {
                  InstanceId = [[Client1_2882]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_2883]],  
                    Class = [[Position]],  
                    x = 36427.60938,  
                    y = -1221.796875,  
                    z = -16.375
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_2870]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_2988]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Start to North Fyros]],  
              Points = {
                {
                  InstanceId = [[Client1_2990]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_2991]],  
                    Class = [[Position]],  
                    x = 36263.875,  
                    y = -1100.046875,  
                    z = -13.453125
                  }
                },  
                {
                  InstanceId = [[Client1_2993]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_2994]],  
                    Class = [[Position]],  
                    x = 36411,  
                    y = -1112.734375,  
                    z = -1.75
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_2987]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_2996]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Start to South Be-ci]],  
              Points = {
                {
                  InstanceId = [[Client1_2998]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_2999]],  
                    Class = [[Position]],  
                    x = 36258.60938,  
                    y = -1106.78125,  
                    z = -12.03125
                  }
                },  
                {
                  InstanceId = [[Client1_3001]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3002]],  
                    Class = [[Position]],  
                    x = 36320.40625,  
                    y = -1260.203125,  
                    z = -0.53125
                  }
                },  
                {
                  InstanceId = [[Client1_3004]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3005]],  
                    Class = [[Position]],  
                    x = 36387.5,  
                    y = -1282.4375,  
                    z = -21
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_2995]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_3010]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[South to North Be-ci]],  
              Points = {
                {
                  InstanceId = [[Client1_3012]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3013]],  
                    Class = [[Position]],  
                    x = 36384.95313,  
                    y = -1274.25,  
                    z = -20.640625
                  }
                },  
                {
                  InstanceId = [[Client1_3015]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3016]],  
                    Class = [[Position]],  
                    x = 36356.6875,  
                    y = -1245.90625,  
                    z = -18
                  }
                },  
                {
                  InstanceId = [[Client1_3018]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3019]],  
                    Class = [[Position]],  
                    x = 36333.0625,  
                    y = -1190.421875,  
                    z = -16.9375
                  }
                },  
                {
                  InstanceId = [[Client1_3021]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3022]],  
                    Class = [[Position]],  
                    x = 36352.98438,  
                    y = -1141,  
                    z = -18.71875
                  }
                },  
                {
                  InstanceId = [[Client1_3024]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3025]],  
                    Class = [[Position]],  
                    x = 36416.04688,  
                    y = -1121.234375,  
                    z = 1.296875
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3009]],  
                Class = [[Position]],  
                x = 0.921875,  
                y = -0.171875,  
                z = -0.0625
              }
            },  
            {
              InstanceId = [[Client1_3027]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[North to South Be-ci]],  
              Points = {
                {
                  InstanceId = [[Client1_3029]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3030]],  
                    Class = [[Position]],  
                    x = 36381.01563,  
                    y = -1123.546875,  
                    z = -20.03125
                  }
                },  
                {
                  InstanceId = [[Client1_3032]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3033]],  
                    Class = [[Position]],  
                    x = 36351.3125,  
                    y = -1136.90625,  
                    z = -16.625
                  }
                },  
                {
                  InstanceId = [[Client1_3035]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3036]],  
                    Class = [[Position]],  
                    x = 36331.53125,  
                    y = -1193.65625,  
                    z = -17.359375
                  }
                },  
                {
                  InstanceId = [[Client1_3038]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3039]],  
                    Class = [[Position]],  
                    x = 36356.04688,  
                    y = -1245.65625,  
                    z = -17.140625
                  }
                },  
                {
                  InstanceId = [[Client1_3041]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3042]],  
                    Class = [[Position]],  
                    x = 36388.57813,  
                    y = -1282.015625,  
                    z = -20.796875
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3026]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_3044]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[South Attack]],  
              Points = {
                {
                  InstanceId = [[Client1_3046]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3047]],  
                    Class = [[Position]],  
                    x = 36397.0625,  
                    y = -1284.0625,  
                    z = -20.703125
                  }
                },  
                {
                  InstanceId = [[Client1_3049]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3050]],  
                    Class = [[Position]],  
                    x = 36422.51563,  
                    y = -1261.40625,  
                    z = -18.296875
                  }
                },  
                {
                  InstanceId = [[Client1_3052]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3053]],  
                    Class = [[Position]],  
                    x = 36441.17188,  
                    y = -1222.171875,  
                    z = -19.765625
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3043]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_3055]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[North Attack]],  
              Points = {
                {
                  InstanceId = [[Client1_3057]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3058]],  
                    Class = [[Position]],  
                    x = 36425.23438,  
                    y = -1123.5625,  
                    z = -3.171875
                  }
                },  
                {
                  InstanceId = [[Client1_3060]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3061]],  
                    Class = [[Position]],  
                    x = 36456.84375,  
                    y = -1137.828125,  
                    z = -17.8125
                  }
                },  
                {
                  InstanceId = [[Client1_3063]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3064]],  
                    Class = [[Position]],  
                    x = 36467.51563,  
                    y = -1155.734375,  
                    z = -17.78125
                  }
                },  
                {
                  InstanceId = [[Client1_3066]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3067]],  
                    Class = [[Position]],  
                    x = 36448.6875,  
                    y = -1198.234375,  
                    z = -17.140625
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3054]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_3116]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Start to North Aby]],  
              Points = {
                {
                  InstanceId = [[Client1_3118]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3119]],  
                    Class = [[Position]],  
                    x = 36264.79688,  
                    y = -1101.46875,  
                    z = -13.046875
                  }
                },  
                {
                  InstanceId = [[Client1_3121]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3122]],  
                    Class = [[Position]],  
                    x = 36417.6875,  
                    y = -1117.546875,  
                    z = -0.484375
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3115]],  
                Class = [[Position]],  
                x = -0.25,  
                y = 0,  
                z = -0.09375
              }
            },  
            {
              InstanceId = [[Client1_3124]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Start to North Matis]],  
              Points = {
                {
                  InstanceId = [[Client1_3126]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3127]],  
                    Class = [[Position]],  
                    x = 36264.03125,  
                    y = -1099.390625,  
                    z = -13.625
                  }
                },  
                {
                  InstanceId = [[Client1_3129]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3130]],  
                    Class = [[Position]],  
                    x = 36411.6875,  
                    y = -1110.140625,  
                    z = -4.453125
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3123]],  
                Class = [[Position]],  
                x = 0,  
                y = 0,  
                z = 0
              }
            },  
            {
              InstanceId = [[Client1_3141]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Start to North Zorai]],  
              Points = {
                {
                  InstanceId = [[Client1_3144]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3145]],  
                    Class = [[Position]],  
                    x = 36264.71875,  
                    y = -1096.734375,  
                    z = -13.59375
                  }
                },  
                {
                  InstanceId = [[Client1_3147]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3148]],  
                    Class = [[Position]],  
                    x = 36411.6875,  
                    y = -1110.140625,  
                    z = -4.453125
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3142]],  
                Class = [[Position]],  
                x = -1.671875,  
                y = -3.609375,  
                z = -0.109375
              }
            },  
            {
              InstanceId = [[Client1_3159]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Start to North Tryker]],  
              Points = {
                {
                  InstanceId = [[Client1_3162]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3163]],  
                    Class = [[Position]],  
                    x = 36264.03125,  
                    y = -1099.390625,  
                    z = -13.625
                  }
                },  
                {
                  InstanceId = [[Client1_3165]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3166]],  
                    Class = [[Position]],  
                    x = 36411.6875,  
                    y = -1110.140625,  
                    z = -4.453125
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3160]],  
                Class = [[Position]],  
                x = -1.390625,  
                y = 1.984375,  
                z = -1.15625
              }
            },  
            {
              InstanceId = [[Client1_3177]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Start to North Be-ci]],  
              Points = {
                {
                  InstanceId = [[Client1_3180]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3181]],  
                    Class = [[Position]],  
                    x = 36264.79688,  
                    y = -1101.46875,  
                    z = -13.046875
                  }
                },  
                {
                  InstanceId = [[Client1_3183]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3184]],  
                    Class = [[Position]],  
                    x = 36417.73438,  
                    y = -1117.609375,  
                    z = 0.84375
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3178]],  
                Class = [[Position]],  
                x = -0.09375,  
                y = -0.859375,  
                z = 0.140625
              }
            },  
            {
              InstanceId = [[Client1_3195]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Start to North Gale]],  
              Points = {
                {
                  InstanceId = [[Client1_3198]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3199]],  
                    Class = [[Position]],  
                    x = 36265.25,  
                    y = -1101.375,  
                    z = -11.96875
                  }
                },  
                {
                  InstanceId = [[Client1_3201]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3202]],  
                    Class = [[Position]],  
                    x = 36417.6875,  
                    y = -1117.546875,  
                    z = -0.484375
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3196]],  
                Class = [[Position]],  
                x = -2.484375,  
                y = -0.15625,  
                z = -0.984375
              }
            },  
            {
              InstanceId = [[Client1_3213]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Start to North Anisti]],  
              Points = {
                {
                  InstanceId = [[Client1_3216]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3217]],  
                    Class = [[Position]],  
                    x = 36265.10938,  
                    y = -1100.53125,  
                    z = -12.1875
                  }
                },  
                {
                  InstanceId = [[Client1_3219]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3220]],  
                    Class = [[Position]],  
                    x = 36417.6875,  
                    y = -1117.546875,  
                    z = -0.484375
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3214]],  
                Class = [[Position]],  
                x = -3.0625,  
                y = -1.515625,  
                z = -1.046875
              }
            },  
            {
              InstanceId = [[Client1_3313]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[North to South Anisti]],  
              Points = {
                {
                  InstanceId = [[Client1_3316]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3317]],  
                    Class = [[Position]],  
                    x = 36381.01563,  
                    y = -1123.546875,  
                    z = -20.03125
                  }
                },  
                {
                  InstanceId = [[Client1_3319]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3320]],  
                    Class = [[Position]],  
                    x = 36351.3125,  
                    y = -1136.90625,  
                    z = -16.625
                  }
                },  
                {
                  InstanceId = [[Client1_3322]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3323]],  
                    Class = [[Position]],  
                    x = 36331.53125,  
                    y = -1193.65625,  
                    z = -17.359375
                  }
                },  
                {
                  InstanceId = [[Client1_3325]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3326]],  
                    Class = [[Position]],  
                    x = 36356.04688,  
                    y = -1245.65625,  
                    z = -17.140625
                  }
                },  
                {
                  InstanceId = [[Client1_3592]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3593]],  
                    Class = [[Position]],  
                    x = 36388.90625,  
                    y = -1288.53125,  
                    z = -21.140625
                  }
                },  
                {
                  InstanceId = [[Client1_3328]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3329]],  
                    Class = [[Position]],  
                    x = 36389.82813,  
                    y = -1287.640625,  
                    z = -21.140625
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3314]],  
                Class = [[Position]],  
                x = -1.34375,  
                y = 0.984375,  
                z = 0.03125
              }
            },  
            {
              InstanceId = [[Client1_3349]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[North to South Aby]],  
              Points = {
                {
                  InstanceId = [[Client1_3352]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3353]],  
                    Class = [[Position]],  
                    x = 36381.01563,  
                    y = -1123.546875,  
                    z = -20.03125
                  }
                },  
                {
                  InstanceId = [[Client1_3355]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3356]],  
                    Class = [[Position]],  
                    x = 36351.3125,  
                    y = -1136.90625,  
                    z = -16.625
                  }
                },  
                {
                  InstanceId = [[Client1_3358]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3359]],  
                    Class = [[Position]],  
                    x = 36331.53125,  
                    y = -1193.65625,  
                    z = -17.359375
                  }
                },  
                {
                  InstanceId = [[Client1_3361]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3362]],  
                    Class = [[Position]],  
                    x = 36356.04688,  
                    y = -1245.65625,  
                    z = -17.140625
                  }
                },  
                {
                  InstanceId = [[Client1_3589]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3590]],  
                    Class = [[Position]],  
                    x = 36392.04688,  
                    y = -1291.28125,  
                    z = -21.0625
                  }
                },  
                {
                  InstanceId = [[Client1_3364]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3365]],  
                    Class = [[Position]],  
                    x = 36392.78125,  
                    y = -1290.703125,  
                    z = -21.0625
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3350]],  
                Class = [[Position]],  
                x = -2.21875,  
                y = 1.765625,  
                z = 0.078125
              }
            },  
            {
              InstanceId = [[Client1_3385]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[North to South Gale]],  
              Points = {
                {
                  InstanceId = [[Client1_3388]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3389]],  
                    Class = [[Position]],  
                    x = 36381.01563,  
                    y = -1123.546875,  
                    z = -20.03125
                  }
                },  
                {
                  InstanceId = [[Client1_3391]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3392]],  
                    Class = [[Position]],  
                    x = 36351.3125,  
                    y = -1136.90625,  
                    z = -16.625
                  }
                },  
                {
                  InstanceId = [[Client1_3394]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3395]],  
                    Class = [[Position]],  
                    x = 36331.53125,  
                    y = -1193.65625,  
                    z = -17.359375
                  }
                },  
                {
                  InstanceId = [[Client1_3397]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3398]],  
                    Class = [[Position]],  
                    x = 36356.04688,  
                    y = -1245.65625,  
                    z = -17.140625
                  }
                },  
                {
                  InstanceId = [[Client1_3586]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3587]],  
                    Class = [[Position]],  
                    x = 36395.32813,  
                    y = -1293.328125,  
                    z = -21.203125
                  }
                },  
                {
                  InstanceId = [[Client1_3400]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3401]],  
                    Class = [[Position]],  
                    x = 36395.8125,  
                    y = -1292.6875,  
                    z = -21.203125
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3386]],  
                Class = [[Position]],  
                x = -3.140625,  
                y = 2.546875,  
                z = 0.203125
              }
            },  
            {
              InstanceId = [[Client1_3421]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[North to South Zorai]],  
              Points = {
                {
                  InstanceId = [[Client1_3424]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3425]],  
                    Class = [[Position]],  
                    x = 36381.01563,  
                    y = -1123.546875,  
                    z = -20.03125
                  }
                },  
                {
                  InstanceId = [[Client1_3427]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3428]],  
                    Class = [[Position]],  
                    x = 36351.3125,  
                    y = -1136.90625,  
                    z = -16.625
                  }
                },  
                {
                  InstanceId = [[Client1_3430]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3431]],  
                    Class = [[Position]],  
                    x = 36331.53125,  
                    y = -1193.65625,  
                    z = -17.359375
                  }
                },  
                {
                  InstanceId = [[Client1_3433]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3434]],  
                    Class = [[Position]],  
                    x = 36356.04688,  
                    y = -1245.65625,  
                    z = -17.140625
                  }
                },  
                {
                  InstanceId = [[Client1_3436]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3437]],  
                    Class = [[Position]],  
                    x = 36387.23438,  
                    y = -1289.375,  
                    z = -21.65625
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3422]],  
                Class = [[Position]],  
                x = -4.125,  
                y = 3.3125,  
                z = 0.453125
              }
            },  
            {
              InstanceId = [[Client1_3457]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[North to South Matis]],  
              Points = {
                {
                  InstanceId = [[Client1_3460]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3461]],  
                    Class = [[Position]],  
                    x = 36381.01563,  
                    y = -1123.546875,  
                    z = -20.03125
                  }
                },  
                {
                  InstanceId = [[Client1_3463]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3464]],  
                    Class = [[Position]],  
                    x = 36351.3125,  
                    y = -1136.90625,  
                    z = -16.625
                  }
                },  
                {
                  InstanceId = [[Client1_3466]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3467]],  
                    Class = [[Position]],  
                    x = 36331.53125,  
                    y = -1193.65625,  
                    z = -17.359375
                  }
                },  
                {
                  InstanceId = [[Client1_3469]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3470]],  
                    Class = [[Position]],  
                    x = 36356.04688,  
                    y = -1245.65625,  
                    z = -17.140625
                  }
                },  
                {
                  InstanceId = [[Client1_3472]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3473]],  
                    Class = [[Position]],  
                    x = 36389.03125,  
                    y = -1285.21875,  
                    z = -22.140625
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3458]],  
                Class = [[Position]],  
                x = -6.21875,  
                y = 4.984375,  
                z = 1.125
              }
            },  
            {
              InstanceId = [[Client1_3493]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[North to South Fyros]],  
              Points = {
                {
                  InstanceId = [[Client1_3496]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3497]],  
                    Class = [[Position]],  
                    x = 36381.01563,  
                    y = -1123.546875,  
                    z = -20.03125
                  }
                },  
                {
                  InstanceId = [[Client1_3499]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3500]],  
                    Class = [[Position]],  
                    x = 36351.3125,  
                    y = -1136.90625,  
                    z = -16.625
                  }
                },  
                {
                  InstanceId = [[Client1_3502]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3503]],  
                    Class = [[Position]],  
                    x = 36331.53125,  
                    y = -1193.65625,  
                    z = -17.359375
                  }
                },  
                {
                  InstanceId = [[Client1_3505]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3506]],  
                    Class = [[Position]],  
                    x = 36356.04688,  
                    y = -1245.65625,  
                    z = -17.140625
                  }
                },  
                {
                  InstanceId = [[Client1_3508]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3509]],  
                    Class = [[Position]],  
                    x = 36387.875,  
                    y = -1287.390625,  
                    z = -21.859375
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3494]],  
                Class = [[Position]],  
                x = -4.96875,  
                y = 4.296875,  
                z = 0.734375
              }
            },  
            {
              InstanceId = [[Client1_3529]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[North to South Tryker]],  
              Points = {
                {
                  InstanceId = [[Client1_3532]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3533]],  
                    Class = [[Position]],  
                    x = 36381.01563,  
                    y = -1123.546875,  
                    z = -20.03125
                  }
                },  
                {
                  InstanceId = [[Client1_3535]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3536]],  
                    Class = [[Position]],  
                    x = 36351.3125,  
                    y = -1136.90625,  
                    z = -16.625
                  }
                },  
                {
                  InstanceId = [[Client1_3538]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3539]],  
                    Class = [[Position]],  
                    x = 36331.53125,  
                    y = -1193.65625,  
                    z = -17.359375
                  }
                },  
                {
                  InstanceId = [[Client1_3541]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3542]],  
                    Class = [[Position]],  
                    x = 36356.04688,  
                    y = -1245.65625,  
                    z = -17.140625
                  }
                },  
                {
                  InstanceId = [[Client1_3544]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3545]],  
                    Class = [[Position]],  
                    x = 36390.07813,  
                    y = -1284.140625,  
                    z = -22.5
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3530]],  
                Class = [[Position]],  
                x = -7.0625,  
                y = 5.78125,  
                z = 1.53125
              }
            },  
            {
              InstanceId = [[Client1_3607]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Start to South Anisti]],  
              Points = {
                {
                  InstanceId = [[Client1_3610]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3611]],  
                    Class = [[Position]],  
                    x = 36258.60938,  
                    y = -1106.78125,  
                    z = -12.03125
                  }
                },  
                {
                  InstanceId = [[Client1_3613]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3614]],  
                    Class = [[Position]],  
                    x = 36320.40625,  
                    y = -1260.203125,  
                    z = -0.53125
                  }
                },  
                {
                  InstanceId = [[Client1_3763]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3764]],  
                    Class = [[Position]],  
                    x = 36387.98438,  
                    y = -1286.953125,  
                    z = -21.234375
                  }
                },  
                {
                  InstanceId = [[Client1_3616]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3617]],  
                    Class = [[Position]],  
                    x = 36389.46875,  
                    y = -1286.53125,  
                    z = -21.21875
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3608]],  
                Class = [[Position]],  
                x = -0.90625,  
                y = -0.3125,  
                z = 0.171875
              }
            },  
            {
              InstanceId = [[Client1_3631]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Start to South Aby]],  
              Points = {
                {
                  InstanceId = [[Client1_3634]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3635]],  
                    Class = [[Position]],  
                    x = 36258.60938,  
                    y = -1106.78125,  
                    z = -12.03125
                  }
                },  
                {
                  InstanceId = [[Client1_3637]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3638]],  
                    Class = [[Position]],  
                    x = 36320.40625,  
                    y = -1260.203125,  
                    z = -0.53125
                  }
                },  
                {
                  InstanceId = [[Client1_3766]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3767]],  
                    Class = [[Position]],  
                    x = 36391.3125,  
                    y = -1288.5625,  
                    z = -21.203125
                  }
                },  
                {
                  InstanceId = [[Client1_3640]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3641]],  
                    Class = [[Position]],  
                    x = 36392.53125,  
                    y = -1288.046875,  
                    z = -21.171875
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3632]],  
                Class = [[Position]],  
                x = -2.125,  
                y = -0.796875,  
                z = 0.1875
              }
            },  
            {
              InstanceId = [[Client1_3655]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Start to South Gale]],  
              Points = {
                {
                  InstanceId = [[Client1_3658]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3659]],  
                    Class = [[Position]],  
                    x = 36258.60938,  
                    y = -1106.78125,  
                    z = -12.03125
                  }
                },  
                {
                  InstanceId = [[Client1_3661]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3662]],  
                    Class = [[Position]],  
                    x = 36320.40625,  
                    y = -1260.203125,  
                    z = -0.53125
                  }
                },  
                {
                  InstanceId = [[Client1_3769]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3770]],  
                    Class = [[Position]],  
                    x = 36395.03125,  
                    y = -1289.53125,  
                    z = -21.0625
                  }
                },  
                {
                  InstanceId = [[Client1_3664]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3665]],  
                    Class = [[Position]],  
                    x = 36395.6875,  
                    y = -1288.890625,  
                    z = -20.890625
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3656]],  
                Class = [[Position]],  
                x = -3.359375,  
                y = -1.296875,  
                z = 0.0625
              }
            },  
            {
              InstanceId = [[Client1_3679]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Start to South Tryker]],  
              Points = {
                {
                  InstanceId = [[Client1_3682]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3683]],  
                    Class = [[Position]],  
                    x = 36258.60938,  
                    y = -1106.78125,  
                    z = -12.03125
                  }
                },  
                {
                  InstanceId = [[Client1_3685]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3686]],  
                    Class = [[Position]],  
                    x = 36320.40625,  
                    y = -1260.203125,  
                    z = -0.53125
                  }
                },  
                {
                  InstanceId = [[Client1_3688]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3689]],  
                    Class = [[Position]],  
                    x = 36386.5,  
                    y = -1285.734375,  
                    z = -21.203125
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3680]],  
                Class = [[Position]],  
                x = -4.40625,  
                y = -1.75,  
                z = 0.09375
              }
            },  
            {
              InstanceId = [[Client1_3703]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Start to South Matis]],  
              Points = {
                {
                  InstanceId = [[Client1_3706]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3707]],  
                    Class = [[Position]],  
                    x = 36258.60938,  
                    y = -1106.78125,  
                    z = -12.03125
                  }
                },  
                {
                  InstanceId = [[Client1_3709]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3710]],  
                    Class = [[Position]],  
                    x = 36320.40625,  
                    y = -1260.203125,  
                    z = -0.53125
                  }
                },  
                {
                  InstanceId = [[Client1_3712]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3713]],  
                    Class = [[Position]],  
                    x = 36387.60938,  
                    y = -1286.890625,  
                    z = -20.90625
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3704]],  
                Class = [[Position]],  
                x = -5.59375,  
                y = -2.28125,  
                z = -0.03125
              }
            },  
            {
              InstanceId = [[Client1_3727]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Start to South Fyros]],  
              Points = {
                {
                  InstanceId = [[Client1_3730]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3731]],  
                    Class = [[Position]],  
                    x = 36258.60938,  
                    y = -1106.78125,  
                    z = -12.03125
                  }
                },  
                {
                  InstanceId = [[Client1_3733]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3734]],  
                    Class = [[Position]],  
                    x = 36320.40625,  
                    y = -1260.203125,  
                    z = -0.53125
                  }
                },  
                {
                  InstanceId = [[Client1_3736]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3737]],  
                    Class = [[Position]],  
                    x = 36389.07813,  
                    y = -1289.328125,  
                    z = -20.359375
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3728]],  
                Class = [[Position]],  
                x = -6.921875,  
                y = -2.796875,  
                z = -0.265625
              }
            },  
            {
              InstanceId = [[Client1_3751]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Start to South Zorai]],  
              Points = {
                {
                  InstanceId = [[Client1_3754]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3755]],  
                    Class = [[Position]],  
                    x = 36258.60938,  
                    y = -1106.78125,  
                    z = -12.03125
                  }
                },  
                {
                  InstanceId = [[Client1_3757]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3758]],  
                    Class = [[Position]],  
                    x = 36320.40625,  
                    y = -1260.203125,  
                    z = -0.53125
                  }
                },  
                {
                  InstanceId = [[Client1_3760]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3761]],  
                    Class = [[Position]],  
                    x = 36390.59375,  
                    y = -1291.578125,  
                    z = -19.890625
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3752]],  
                Class = [[Position]],  
                x = -8.28125,  
                y = -3.390625,  
                z = -0.4375
              }
            },  
            {
              InstanceId = [[Client1_3925]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[South to North Gale]],  
              Points = {
                {
                  InstanceId = [[Client1_3928]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3929]],  
                    Class = [[Position]],  
                    x = 36384.95313,  
                    y = -1274.25,  
                    z = -20.640625
                  }
                },  
                {
                  InstanceId = [[Client1_3931]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3932]],  
                    Class = [[Position]],  
                    x = 36356.6875,  
                    y = -1245.90625,  
                    z = -18
                  }
                },  
                {
                  InstanceId = [[Client1_3934]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3935]],  
                    Class = [[Position]],  
                    x = 36333.0625,  
                    y = -1190.421875,  
                    z = -16.9375
                  }
                },  
                {
                  InstanceId = [[Client1_3937]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3938]],  
                    Class = [[Position]],  
                    x = 36352.98438,  
                    y = -1141,  
                    z = -18.71875
                  }
                },  
                {
                  InstanceId = [[Client1_3940]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3941]],  
                    Class = [[Position]],  
                    x = 36414.10938,  
                    y = -1123.578125,  
                    z = 1.1875
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3926]],  
                Class = [[Position]],  
                x = 1.234375,  
                y = 0.671875,  
                z = 0.0625
              }
            },  
            {
              InstanceId = [[Client1_3961]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[South to North Ani]],  
              Points = {
                {
                  InstanceId = [[Client1_3964]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3965]],  
                    Class = [[Position]],  
                    x = 36384.95313,  
                    y = -1274.25,  
                    z = -20.640625
                  }
                },  
                {
                  InstanceId = [[Client1_3967]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3968]],  
                    Class = [[Position]],  
                    x = 36356.6875,  
                    y = -1245.90625,  
                    z = -18
                  }
                },  
                {
                  InstanceId = [[Client1_3970]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3971]],  
                    Class = [[Position]],  
                    x = 36333.0625,  
                    y = -1190.421875,  
                    z = -16.9375
                  }
                },  
                {
                  InstanceId = [[Client1_3973]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3974]],  
                    Class = [[Position]],  
                    x = 36352.98438,  
                    y = -1141,  
                    z = -18.71875
                  }
                },  
                {
                  InstanceId = [[Client1_3976]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_3977]],  
                    Class = [[Position]],  
                    x = 36412.35938,  
                    y = -1121.6875,  
                    z = 1.4375
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3962]],  
                Class = [[Position]],  
                x = 2,  
                y = 0.90625,  
                z = 0.0625
              }
            },  
            {
              InstanceId = [[Client1_3997]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[South to North Aby]],  
              Points = {
                {
                  InstanceId = [[Client1_4000]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4001]],  
                    Class = [[Position]],  
                    x = 36384.95313,  
                    y = -1274.25,  
                    z = -20.640625
                  }
                },  
                {
                  InstanceId = [[Client1_4003]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4004]],  
                    Class = [[Position]],  
                    x = 36356.6875,  
                    y = -1245.90625,  
                    z = -18
                  }
                },  
                {
                  InstanceId = [[Client1_4006]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4007]],  
                    Class = [[Position]],  
                    x = 36333.0625,  
                    y = -1190.421875,  
                    z = -16.9375
                  }
                },  
                {
                  InstanceId = [[Client1_4009]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4010]],  
                    Class = [[Position]],  
                    x = 36352.98438,  
                    y = -1141,  
                    z = -18.71875
                  }
                },  
                {
                  InstanceId = [[Client1_4012]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4013]],  
                    Class = [[Position]],  
                    x = 36413.25,  
                    y = -1125.6875,  
                    z = 1.3125
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_3998]],  
                Class = [[Position]],  
                x = 2.359375,  
                y = 1.25,  
                z = 0.125
              }
            },  
            {
              InstanceId = [[Client1_4033]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[South to North Fyros]],  
              Points = {
                {
                  InstanceId = [[Client1_4036]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4037]],  
                    Class = [[Position]],  
                    x = 36384.95313,  
                    y = -1274.25,  
                    z = -20.640625
                  }
                },  
                {
                  InstanceId = [[Client1_4039]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4040]],  
                    Class = [[Position]],  
                    x = 36356.6875,  
                    y = -1245.90625,  
                    z = -18
                  }
                },  
                {
                  InstanceId = [[Client1_4042]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4043]],  
                    Class = [[Position]],  
                    x = 36333.0625,  
                    y = -1190.421875,  
                    z = -16.9375
                  }
                },  
                {
                  InstanceId = [[Client1_4045]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4046]],  
                    Class = [[Position]],  
                    x = 36352.98438,  
                    y = -1141,  
                    z = -18.71875
                  }
                },  
                {
                  InstanceId = [[Client1_4048]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4049]],  
                    Class = [[Position]],  
                    x = 36408.82813,  
                    y = -1125.5,  
                    z = -2.0625
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_4034]],  
                Class = [[Position]],  
                x = -1.390625,  
                y = 0.390625,  
                z = 0.171875
              }
            },  
            {
              InstanceId = [[Client1_4069]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[South to North Zorai]],  
              Points = {
                {
                  InstanceId = [[Client1_4072]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4073]],  
                    Class = [[Position]],  
                    x = 36384.95313,  
                    y = -1274.25,  
                    z = -20.640625
                  }
                },  
                {
                  InstanceId = [[Client1_4075]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4076]],  
                    Class = [[Position]],  
                    x = 36356.6875,  
                    y = -1245.90625,  
                    z = -18
                  }
                },  
                {
                  InstanceId = [[Client1_4078]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4079]],  
                    Class = [[Position]],  
                    x = 36333.0625,  
                    y = -1190.421875,  
                    z = -16.9375
                  }
                },  
                {
                  InstanceId = [[Client1_4081]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4082]],  
                    Class = [[Position]],  
                    x = 36352.98438,  
                    y = -1141,  
                    z = -18.71875
                  }
                },  
                {
                  InstanceId = [[Client1_4084]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4085]],  
                    Class = [[Position]],  
                    x = 36409.17188,  
                    y = -1127.234375,  
                    z = -2.5
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_4070]],  
                Class = [[Position]],  
                x = -1.875,  
                y = -0.234375,  
                z = 0.078125
              }
            },  
            {
              InstanceId = [[Client1_4105]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[South to North Matis]],  
              Points = {
                {
                  InstanceId = [[Client1_4108]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4109]],  
                    Class = [[Position]],  
                    x = 36384.95313,  
                    y = -1274.25,  
                    z = -20.640625
                  }
                },  
                {
                  InstanceId = [[Client1_4111]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4112]],  
                    Class = [[Position]],  
                    x = 36356.6875,  
                    y = -1245.90625,  
                    z = -18
                  }
                },  
                {
                  InstanceId = [[Client1_4114]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4115]],  
                    Class = [[Position]],  
                    x = 36333.0625,  
                    y = -1190.421875,  
                    z = -16.9375
                  }
                },  
                {
                  InstanceId = [[Client1_4117]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4118]],  
                    Class = [[Position]],  
                    x = 36352.98438,  
                    y = -1141,  
                    z = -18.71875
                  }
                },  
                {
                  InstanceId = [[Client1_4120]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4121]],  
                    Class = [[Position]],  
                    x = 36407.51563,  
                    y = -1124.125,  
                    z = -2.21875
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_4106]],  
                Class = [[Position]],  
                x = -0.78125,  
                y = 0.59375,  
                z = 0.15625
              }
            },  
            {
              InstanceId = [[Client1_4141]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[South to North Tryker]],  
              Points = {
                {
                  InstanceId = [[Client1_4144]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4145]],  
                    Class = [[Position]],  
                    x = 36384.95313,  
                    y = -1274.25,  
                    z = -20.640625
                  }
                },  
                {
                  InstanceId = [[Client1_4147]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4148]],  
                    Class = [[Position]],  
                    x = 36356.6875,  
                    y = -1245.90625,  
                    z = -18
                  }
                },  
                {
                  InstanceId = [[Client1_4150]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4151]],  
                    Class = [[Position]],  
                    x = 36333.0625,  
                    y = -1190.421875,  
                    z = -16.9375
                  }
                },  
                {
                  InstanceId = [[Client1_4153]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4154]],  
                    Class = [[Position]],  
                    x = 36352.98438,  
                    y = -1141,  
                    z = -18.71875
                  }
                },  
                {
                  InstanceId = [[Client1_4156]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4157]],  
                    Class = [[Position]],  
                    x = 36407,  
                    y = -1129.90625,  
                    z = -2.859375
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_4142]],  
                Class = [[Position]],  
                x = -0.203125,  
                y = 0.5625,  
                z = 0.125
              }
            },  
            {
              InstanceId = [[Client1_4248]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[South to North Fyros split]],  
              Points = {
                {
                  InstanceId = [[Client1_4251]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4252]],  
                    Class = [[Position]],  
                    x = 36384.95313,  
                    y = -1274.25,  
                    z = -20.640625
                  }
                },  
                {
                  InstanceId = [[Client1_4254]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4255]],  
                    Class = [[Position]],  
                    x = 36356.6875,  
                    y = -1245.90625,  
                    z = -18
                  }
                },  
                {
                  InstanceId = [[Client1_4257]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4258]],  
                    Class = [[Position]],  
                    x = 36333.0625,  
                    y = -1190.421875,  
                    z = -16.9375
                  }
                },  
                {
                  InstanceId = [[Client1_4260]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4261]],  
                    Class = [[Position]],  
                    x = 36352.98438,  
                    y = -1141,  
                    z = -18.71875
                  }
                },  
                {
                  InstanceId = [[Client1_4263]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4264]],  
                    Class = [[Position]],  
                    x = 36408.82813,  
                    y = -1125.5,  
                    z = -2.0625
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_4249]],  
                Class = [[Position]],  
                x = -1.5625,  
                y = 0.484375,  
                z = 0.234375
              }
            },  
            {
              InstanceId = [[Client1_4287]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[North to South Anisti Split]],  
              Points = {
                {
                  InstanceId = [[Client1_4290]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4291]],  
                    Class = [[Position]],  
                    x = 36381.01563,  
                    y = -1123.546875,  
                    z = -20.03125
                  }
                },  
                {
                  InstanceId = [[Client1_4293]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4294]],  
                    Class = [[Position]],  
                    x = 36351.3125,  
                    y = -1136.90625,  
                    z = -16.625
                  }
                },  
                {
                  InstanceId = [[Client1_4296]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4297]],  
                    Class = [[Position]],  
                    x = 36331.53125,  
                    y = -1193.65625,  
                    z = -17.359375
                  }
                },  
                {
                  InstanceId = [[Client1_4299]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4300]],  
                    Class = [[Position]],  
                    x = 36356.04688,  
                    y = -1245.65625,  
                    z = -17.140625
                  }
                },  
                {
                  InstanceId = [[Client1_4302]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4303]],  
                    Class = [[Position]],  
                    x = 36388.90625,  
                    y = -1288.53125,  
                    z = -21.140625
                  }
                },  
                {
                  InstanceId = [[Client1_4305]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4306]],  
                    Class = [[Position]],  
                    x = 36389.82813,  
                    y = -1287.640625,  
                    z = -21.140625
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_4288]],  
                Class = [[Position]],  
                x = -0.96875,  
                y = 0.890625,  
                z = -0.109375
              }
            },  
            {
              InstanceId = [[Client1_4408]],  
              Class = [[Road]],  
              InheritPos = 1,  
              Name = [[Start to South Anisti split]],  
              Points = {
                {
                  InstanceId = [[Client1_4411]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4412]],  
                    Class = [[Position]],  
                    x = 36258.60938,  
                    y = -1106.78125,  
                    z = -12.03125
                  }
                },  
                {
                  InstanceId = [[Client1_4414]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4415]],  
                    Class = [[Position]],  
                    x = 36320.40625,  
                    y = -1260.203125,  
                    z = -0.53125
                  }
                },  
                {
                  InstanceId = [[Client1_4417]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4418]],  
                    Class = [[Position]],  
                    x = 36387.98438,  
                    y = -1286.953125,  
                    z = -21.234375
                  }
                },  
                {
                  InstanceId = [[Client1_4420]],  
                  Class = [[WayPoint]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4421]],  
                    Class = [[Position]],  
                    x = 36389.46875,  
                    y = -1286.53125,  
                    z = -21.21875
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_4409]],  
                Class = [[Position]],  
                x = -0.96875,  
                y = 0.15625,  
                z = 0.15625
              }
            },  
            {
              InstanceId = [[Client1_4563]],  
              Class = [[Npc]],  
              Angle = -2.5625,  
              Base = [[palette.entities.botobjects.totem_kami]],  
              InheritPos = 1,  
              Name = [[kami totem 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4561]],  
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
                InstanceId = [[Client1_4564]],  
                Class = [[Position]],  
                x = 32100.42188,  
                y = -1370.359375,  
                z = -27.59375
              }
            },  
            {
              InstanceId = [[Client1_4567]],  
              Class = [[Npc]],  
              Angle = 3.09375,  
              Base = [[palette.entities.botobjects.kami_hut]],  
              InheritPos = 1,  
              Name = [[kami hut 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4565]],  
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
                InstanceId = [[Client1_4568]],  
                Class = [[Position]],  
                x = 32107.34375,  
                y = -1377.4375,  
                z = -27.40625
              }
            },  
            {
              InstanceId = [[Client1_4669]],  
              Class = [[Region]],  
              InheritPos = 1,  
              Name = [[Place 8]],  
              Points = {
                {
                  InstanceId = [[Client1_4671]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4672]],  
                    Class = [[Position]],  
                    x = 32100.25,  
                    y = -1379.765625,  
                    z = -27.9375
                  }
                },  
                {
                  InstanceId = [[Client1_4674]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4675]],  
                    Class = [[Position]],  
                    x = 32094.39063,  
                    y = -1381.859375,  
                    z = -27.609375
                  }
                },  
                {
                  InstanceId = [[Client1_4677]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4678]],  
                    Class = [[Position]],  
                    x = 32094.4375,  
                    y = -1403.21875,  
                    z = -26.65625
                  }
                },  
                {
                  InstanceId = [[Client1_4680]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4681]],  
                    Class = [[Position]],  
                    x = 32117.375,  
                    y = -1396.859375,  
                    z = -25.359375
                  }
                },  
                {
                  InstanceId = [[Client1_4683]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4684]],  
                    Class = [[Position]],  
                    x = 32116.5625,  
                    y = -1386.609375,  
                    z = -26.09375
                  }
                },  
                {
                  InstanceId = [[Client1_4686]],  
                  Class = [[RegionVertex]],  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_4687]],  
                    Class = [[Position]],  
                    x = 32104.9375,  
                    y = -1382.59375,  
                    z = -27.421875
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_4668]],  
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
        InstanceId = [[Client1_7]],  
        Class = [[Position]],  
        x = 0,  
        y = 0,  
        z = 0
      }
    },  
    {
      InstanceId = [[Client1_12]],  
      Class = [[Act]],  
      Version = 6,  
      InheritPos = 1,  
      LocationId = [[Client1_14]],  
      ManualWeather = 1,  
      Name = [[Act 1]],  
      Season = 0,  
      ShortDescription = [[Farmer Nidera's farm has been attacked by the Kitin, help him out!]],  
      Title = [[]],  
      WeatherValue = 0,  
      ActivitiesIds = {
      },  
      Behavior = {
        InstanceId = [[Client1_10]],  
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
              InstanceId = [[Client1_17]],  
              Class = [[NpcCustom]],  
              Angle = -3.09375,  
              ArmColor = 1,  
              ArmModel = 6717230,  
              Base = [[palette.entities.npcs.civils.m_civil_220]],  
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
              InheritPos = 1,  
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
              Name = [[Trigio Nidera]],  
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
                InstanceId = [[Client1_15]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_858]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_884]],  
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
                    InstanceId = [[Client1_887]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_888]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_848]]),  
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
                InstanceId = [[Client1_18]],  
                Class = [[Position]],  
                x = 27084.76563,  
                y = -12939.9375,  
                z = -19.078125
              }
            },  
            {
              InstanceId = [[Client1_21]],  
              Class = [[NpcCustom]],  
              Angle = -1.59375,  
              ArmColor = 4,  
              ArmModel = 6714926,  
              Base = [[palette.entities.npcs.civils.m_civil_220]],  
              EyesColor = 5,  
              FeetColor = 4,  
              FeetModel = 6715950,  
              GabaritArmsWidth = 8,  
              GabaritBreastSize = 8,  
              GabaritHeight = 0,  
              GabaritLegsWidth = 8,  
              GabaritTorsoWidth = 3,  
              HairColor = 2,  
              HairType = 5622574,  
              HandsColor = 4,  
              HandsModel = 6713902,  
              InheritPos = 1,  
              JacketColor = 4,  
              JacketModel = 6715438,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 3,  
              MorphTarget2 = 5,  
              MorphTarget3 = 2,  
              MorphTarget4 = 5,  
              MorphTarget5 = 5,  
              MorphTarget6 = 6,  
              MorphTarget7 = 3,  
              MorphTarget8 = 7,  
              Name = [[Gine Nidera]],  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_civil_light_melee_blunt_f2.creature]],  
              SheetClient = [[basic_matis_female.creature]],  
              Speed = 0,  
              Tattoo = 16,  
              TrouserColor = 4,  
              TrouserModel = 6714414,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_19]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_860]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_861]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Stand Still]],  
                        ActivityZoneId = r2.RefId([[]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_885]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_886]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_848]]),  
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
                InstanceId = [[Client1_22]],  
                Class = [[Position]],  
                x = 27084.23438,  
                y = -12938.82813,  
                z = -19.046875
              }
            }
          }
        },  
        {
          InstanceId = [[Client1_47]],  
          Class = [[EasterEgg]],  
          Active = 1,  
          Base = [[palette.entities.botobjects.chest_wisdom_std_sel]],  
          InheritPos = 1,  
          Item1Id = r2.RefId([[Client1_39]]),  
          Item1Qty = 1,  
          Item2Id = r2.RefId([[]]),  
          Item2Qty = 0,  
          Item3Id = r2.RefId([[]]),  
          Item3Qty = 0,  
          ItemNumber = 0,  
          Name = [[Mektoub Food Crate]],  
          _Seed = 1154277422,  
          Behavior = {
            InstanceId = [[Client1_48]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
          },  
          Position = {
            InstanceId = [[Client1_49]],  
            Class = [[Position]],  
            x = 27138.96875,  
            y = -12650.84375,  
            z = -9.328125
          }
        },  
        {
          InstanceId = [[Client1_51]],  
          Class = [[RequestItem]],  
          Active = 1,  
          Base = [[palette.entities.botobjects.bot_request_item]],  
          ContextualText = [[Give the Mektoub Food to <mission_giver>]],  
          InheritPos = 1,  
          Item1Id = r2.RefId([[Client1_39]]),  
          Item1Qty = 3,  
          Item2Id = r2.RefId([[]]),  
          Item2Qty = 0,  
          Item3Id = r2.RefId([[]]),  
          Item3Qty = 0,  
          ItemNumber = 0,  
          MissionGiver = r2.RefId([[Client1_17]]),  
          MissionSucceedText = [[Thank you so much!  You are welcome to flee with my wife and our Mektoubs through the Prime Roots to safety.]],  
          MissionText = [[Hi there stranger, can you help us?  My farm was attacked by the Kitin, my wife and I escaped with some of our stock.  We need food for the mektoubs, could you gather three <qt1> <item1> from my farm please? Be careful, its dangerous!]],  
          Name = [[Mission: Request Item 2]],  
          Repeatable = 0,  
          WaitValidationText = [[Please my mektoub are hungry, can you give me the  <qt1> <item1>?]],  
          _Seed = 1154277457,  
          Behavior = {
            InstanceId = [[Client1_52]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_823]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_826]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_818]]),  
                    Action = {
                      InstanceId = [[Client1_825]],  
                      Class = [[ActionType]],  
                      Type = [[starts dialog]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_824]],  
                  Class = [[EventType]],  
                  Type = [[succeeded]],  
                  Value = r2.RefId([[]])
                }
              },  
              {
                InstanceId = [[Client1_876]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_879]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_121]]),  
                    Action = {
                      InstanceId = [[Client1_878]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_862]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_881]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_21]]),  
                    Action = {
                      InstanceId = [[Client1_880]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_885]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_883]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_17]]),  
                    Action = {
                      InstanceId = [[Client1_882]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_887]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_877]],  
                  Class = [[EventType]],  
                  Type = [[succeeded]],  
                  Value = r2.RefId([[]])
                }
              },  
              {
                InstanceId = [[Client1_914]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_917]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_910]]),  
                    Action = {
                      InstanceId = [[Client1_916]],  
                      Class = [[ActionType]],  
                      Type = [[Activate]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_915]],  
                  Class = [[EventType]],  
                  Type = [[succeeded]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
          },  
          Position = {
            InstanceId = [[Client1_53]],  
            Class = [[Position]],  
            x = 27083.29688,  
            y = -12946,  
            z = -19.265625
          }
        },  
        {
          InstanceId = [[Client1_55]],  
          Class = [[EasterEgg]],  
          Active = 1,  
          Base = [[palette.entities.botobjects.chest_wisdom_std_sel]],  
          InheritPos = 1,  
          Item1Id = r2.RefId([[Client1_39]]),  
          Item1Qty = 1,  
          Item2Id = r2.RefId([[]]),  
          Item2Qty = 0,  
          Item3Id = r2.RefId([[]]),  
          Item3Qty = 0,  
          ItemNumber = 0,  
          Name = [[Mektoub Food Crate]],  
          _Seed = 1154278138,  
          Behavior = {
            InstanceId = [[Client1_56]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
          },  
          Position = {
            InstanceId = [[Client1_57]],  
            Class = [[Position]],  
            x = 27172.70313,  
            y = -12638.71875,  
            z = -8.984375
          }
        },  
        {
          InstanceId = [[Client1_58]],  
          Class = [[EasterEgg]],  
          Active = 1,  
          Base = [[palette.entities.botobjects.chest_wisdom_std_sel]],  
          InheritPos = 1,  
          Item1Id = r2.RefId([[Client1_39]]),  
          Item1Qty = 1,  
          Item2Id = r2.RefId([[]]),  
          Item2Qty = 0,  
          Item3Id = r2.RefId([[]]),  
          Item3Qty = 0,  
          ItemNumber = 0,  
          Name = [[Mektoub Food Crate]],  
          _Seed = 1154278152,  
          Behavior = {
            InstanceId = [[Client1_59]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
          },  
          Position = {
            InstanceId = [[Client1_60]],  
            Class = [[Position]],  
            x = 27153.8125,  
            y = -12612.3125,  
            z = -9.921875
          }
        },  
        {
          InstanceId = [[Client1_406]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Group 2]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_404]],  
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
              InstanceId = [[Client1_392]],  
              Class = [[NpcCreature]],  
              Angle = -1.116427302,  
              Base = [[palette.entities.creatures.ckdff1]],  
              InheritPos = 1,  
              Name = [[Gruesome Kincher]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_390]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_453]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_454]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Patrol]],  
                        ActivityZoneId = r2.RefId([[Client1_438]]),  
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
                InstanceId = [[Client1_393]],  
                Class = [[Position]],  
                x = 27109.71875,  
                y = -12676.67188,  
                z = -13.8125
              }
            },  
            {
              InstanceId = [[Client1_401]],  
              Class = [[NpcCreature]],  
              Angle = -1.116427302,  
              Base = [[palette.entities.creatures.ckdff1]],  
              InheritPos = 1,  
              Name = [[Gruesome Kincher]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_402]],  
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
                InstanceId = [[Client1_403]],  
                Class = [[Position]],  
                x = 27112.51563,  
                y = -12680.82813,  
                z = -14.890625
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_405]],  
            Class = [[Position]],  
            x = -56.890625,  
            y = -51.859375,  
            z = -5.71875
          }
        },  
        {
          InstanceId = [[Client1_424]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Group 3]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_436]],  
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
              InstanceId = [[Client1_428]],  
              Class = [[NpcCreature]],  
              Angle = -1.116427302,  
              Base = [[palette.entities.creatures.ckdff1]],  
              InheritPos = 1,  
              Name = [[Gruesome Kincher]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_429]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_451]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_452]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Patrol]],  
                        ActivityZoneId = r2.RefId([[Client1_438]]),  
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
                InstanceId = [[Client1_430]],  
                Class = [[Position]],  
                x = 27108.9375,  
                y = -12696.71875,  
                z = -17.453125
              }
            },  
            {
              InstanceId = [[Client1_433]],  
              Class = [[NpcCreature]],  
              Angle = -1.116427302,  
              Base = [[palette.entities.creatures.ckdff1]],  
              InheritPos = 1,  
              Name = [[Gruesome Kincher]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_434]],  
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
                InstanceId = [[Client1_435]],  
                Class = [[Position]],  
                x = 27109.84375,  
                y = -12692.60938,  
                z = -17.609375
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_425]],  
            Class = [[Position]],  
            x = -5.421875,  
            y = -22.53125,  
            z = -1.609375
          }
        },  
        {
          InstanceId = [[Client1_511]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Group 4]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_509]],  
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
              InstanceId = [[Client1_476]],  
              Class = [[NpcCreature]],  
              Angle = -0.7444283366,  
              Base = [[palette.entities.creatures.ckdff4]],  
              InheritPos = 1,  
              Name = [[Great Kincher]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_477]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_532]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_533]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Guard Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_513]]),  
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
                InstanceId = [[Client1_478]],  
                Class = [[Position]],  
                x = 27100.20313,  
                y = -12676.40625,  
                z = -13.6875
              }
            },  
            {
              InstanceId = [[Client1_506]],  
              Class = [[NpcCreature]],  
              Angle = -0.7444283366,  
              Base = [[palette.entities.creatures.ckdff4]],  
              InheritPos = 1,  
              Name = [[Great Kincher]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_507]],  
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
                InstanceId = [[Client1_508]],  
                Class = [[Position]],  
                x = 27107.07813,  
                y = -12680.54688,  
                z = -14.78125
              }
            },  
            {
              InstanceId = [[Client1_466]],  
              Class = [[NpcCreature]],  
              Angle = -0.7444283366,  
              Base = [[palette.entities.creatures.ckdff4]],  
              InheritPos = 1,  
              Name = [[Great Kincher]],  
              NoRespawn = 1,  
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
                InstanceId = [[Client1_468]],  
                Class = [[Position]],  
                x = 27103.95313,  
                y = -12666.48438,  
                z = -11.71875
              }
            },  
            {
              InstanceId = [[Client1_457]],  
              Class = [[NpcCreature]],  
              Angle = -0.7444283366,  
              Base = [[palette.entities.creatures.ckdff4]],  
              InheritPos = 1,  
              Name = [[Great Kincher]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_455]],  
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
                InstanceId = [[Client1_458]],  
                Class = [[Position]],  
                x = 27092.54688,  
                y = -12671.28125,  
                z = -12.71875
              }
            },  
            {
              InstanceId = [[Client1_486]],  
              Class = [[NpcCreature]],  
              Angle = -0.7444283366,  
              Base = [[palette.entities.creatures.ckdff4]],  
              InheritPos = 1,  
              Name = [[Great Kincher]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_487]],  
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
                InstanceId = [[Client1_488]],  
                Class = [[Position]],  
                x = 27096.0625,  
                y = -12660.375,  
                z = -11.328125
              }
            },  
            {
              InstanceId = [[Client1_496]],  
              Class = [[NpcCreature]],  
              Angle = -0.7444283366,  
              Base = [[palette.entities.creatures.ckdff4]],  
              InheritPos = 1,  
              Name = [[Great Kincher]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_497]],  
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
                InstanceId = [[Client1_498]],  
                Class = [[Position]],  
                x = 27112.34375,  
                y = -12672.21875,  
                z = -12.765625
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_510]],  
            Class = [[Position]],  
            x = 52.65625,  
            y = 40.15625,  
            z = 4.671875
          }
        },  
        {
          InstanceId = [[Client1_556]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Group 5]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_554]],  
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
              InstanceId = [[Client1_541]],  
              Class = [[NpcCreature]],  
              Angle = -2.716780186,  
              Base = [[palette.entities.creatures.chhfd3]],  
              InheritPos = 1,  
              Name = [[Domesticated Mektoub]],  
              NoRespawn = 1,  
              PlayerAttackable = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_542]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_812]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_815]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_556]]),  
                        Action = {
                          InstanceId = [[Client1_814]],  
                          Class = [[ActionType]],  
                          Type = [[Kill]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_813]],  
                      Class = [[EventType]],  
                      Type = [[end of activity sequence]],  
                      Value = r2.RefId([[Client1_806]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_806]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_808]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Rest In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_513]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      },  
                      {
                        InstanceId = [[Client1_809]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Feed In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_513]]),  
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
                    InstanceId = [[Client1_810]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                    }
                  }
                },  
                ChatSequences = {
                }
              },  
              Position = {
                InstanceId = [[Client1_543]],  
                Class = [[Position]],  
                x = 27162.10938,  
                y = -12660.15625,  
                z = -11.25
              }
            },  
            {
              InstanceId = [[Client1_551]],  
              Class = [[NpcCreature]],  
              Angle = -2.716780186,  
              Base = [[palette.entities.creatures.chhfd3]],  
              InheritPos = 1,  
              Name = [[Domesticated Mektoub]],  
              NoRespawn = 1,  
              PlayerAttackable = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_552]],  
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
                InstanceId = [[Client1_553]],  
                Class = [[Position]],  
                x = 27165.98438,  
                y = -12660.28125,  
                z = -11.5
              }
            },  
            {
              InstanceId = [[Client1_564]],  
              Class = [[NpcCreature]],  
              Angle = -2.716780186,  
              Base = [[palette.entities.creatures.chhfd3]],  
              InheritPos = 1,  
              Name = [[Domesticated Mektoub]],  
              NoRespawn = 1,  
              PlayerAttackable = 0,  
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
                InstanceId = [[Client1_566]],  
                Class = [[Position]],  
                x = 27157.29688,  
                y = -12659.95313,  
                z = -10.984375
              }
            },  
            {
              InstanceId = [[Client1_616]],  
              Class = [[NpcCreature]],  
              Angle = -2.716780186,  
              Base = [[palette.entities.creatures.chhfd3]],  
              InheritPos = 1,  
              Name = [[Domesticated Mektoub]],  
              NoRespawn = 1,  
              PlayerAttackable = 0,  
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
                InstanceId = [[Client1_618]],  
                Class = [[Position]],  
                x = 27164.70313,  
                y = -12664.21875,  
                z = -12.21875
              }
            },  
            {
              InstanceId = [[Client1_647]],  
              Class = [[NpcCreature]],  
              Angle = -2.716780186,  
              Base = [[palette.entities.creatures.chhfd3]],  
              InheritPos = 1,  
              Name = [[Domesticated Mektoub]],  
              NoRespawn = 1,  
              PlayerAttackable = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_648]],  
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
                InstanceId = [[Client1_649]],  
                Class = [[Position]],  
                x = 27161.82813,  
                y = -12663.26563,  
                z = -11.890625
              }
            },  
            {
              InstanceId = [[Client1_660]],  
              Class = [[NpcCreature]],  
              Angle = -2.716780186,  
              Base = [[palette.entities.creatures.chhfd3]],  
              InheritPos = 1,  
              Name = [[Domesticated Mektoub]],  
              NoRespawn = 1,  
              PlayerAttackable = 0,  
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
                InstanceId = [[Client1_662]],  
                Class = [[Position]],  
                x = 27169.26563,  
                y = -12662.65625,  
                z = -12.34375
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_555]],  
            Class = [[Position]],  
            x = 0,  
            y = 0,  
            z = 0
          }
        },  
        {
          InstanceId = [[Client1_818]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Act 1 end]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_816]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_819]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_820]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_821]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_817]],  
            Class = [[Position]],  
            x = 27077.59375,  
            y = -12949.875,  
            z = -19
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_829]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 1,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Act 1 Start]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_827]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_830]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 2,  
              Actions = {
                {
                  InstanceId = [[Client1_831]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_846]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_828]],  
            Class = [[Position]],  
            x = 27082.15625,  
            y = -12947.64063,  
            z = -19
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_910]],  
          Class = [[Timer]],  
          Active = 0,  
          Base = [[palette.entities.botobjects.timer]],  
          Cyclic = 0,  
          InheritPos = 1,  
          Minutes = 0,  
          Name = [[Time to act 2]],  
          Secondes = 7,  
          Behavior = {
            InstanceId = [[Client1_911]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_919]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_922]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_891]]),  
                    Action = {
                      InstanceId = [[Client1_921]],  
                      Class = [[ActionType]],  
                      Type = [[Start Act]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_920]],  
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
            InstanceId = [[Client1_912]],  
            Class = [[Position]],  
            x = 27079.4375,  
            y = -12947.85938,  
            z = -19
          }
        },  
        {
          InstanceId = [[Client1_121]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Rescued Mektoubs]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_119]],  
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
              InstanceId = [[Client1_107]],  
              Class = [[NpcCreature]],  
              Angle = -2.716780186,  
              Base = [[palette.entities.creatures.chhfd3]],  
              InheritPos = 1,  
              Name = [[Domesticated Mektoub]],  
              PlayerAttackable = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_105]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_190]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_191]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Rest In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_174]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      },  
                      {
                        InstanceId = [[Client1_192]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Feed In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_174]]),  
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
                    InstanceId = [[Client1_862]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_874]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Rest In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_864]]),  
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
                InstanceId = [[Client1_108]],  
                Class = [[Position]],  
                x = 27088.84375,  
                y = -12939.65625,  
                z = -19.015625
              }
            },  
            {
              InstanceId = [[Client1_116]],  
              Class = [[NpcCreature]],  
              Angle = -2.716780186,  
              Base = [[palette.entities.creatures.chhfd3]],  
              InheritPos = 1,  
              Name = [[Domesticated Mektoub]],  
              PlayerAttackable = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_117]],  
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
                InstanceId = [[Client1_118]],  
                Class = [[Position]],  
                x = 27088.32813,  
                y = -12936.875,  
                z = -19.03125
              }
            },  
            {
              InstanceId = [[Client1_129]],  
              Class = [[NpcCreature]],  
              Angle = -2.716780186,  
              Base = [[palette.entities.creatures.chhfd3]],  
              InheritPos = 1,  
              Name = [[Domesticated Mektoub]],  
              PlayerAttackable = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_130]],  
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
                InstanceId = [[Client1_132]],  
                Class = [[Position]],  
                x = 27090.3125,  
                y = -12942.65625,  
                z = -19.0625
              }
            },  
            {
              InstanceId = [[Client1_164]],  
              Class = [[NpcCreature]],  
              Angle = -2.716780186,  
              Base = [[palette.entities.creatures.chhfd3]],  
              InheritPos = 1,  
              Name = [[Domesticated Mektoub]],  
              PlayerAttackable = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_165]],  
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
                InstanceId = [[Client1_166]],  
                Class = [[Position]],  
                x = 27093.5,  
                y = -12935.78125,  
                z = -19
              }
            },  
            {
              InstanceId = [[Client1_159]],  
              Class = [[NpcCreature]],  
              Angle = -2.716780186,  
              Base = [[palette.entities.creatures.chhfd3]],  
              InheritPos = 1,  
              Name = [[Domesticated Mektoub]],  
              PlayerAttackable = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_160]],  
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
                InstanceId = [[Client1_161]],  
                Class = [[Position]],  
                x = 27094.82813,  
                y = -12938.64063,  
                z = -19.03125
              }
            },  
            {
              InstanceId = [[Client1_169]],  
              Class = [[NpcCreature]],  
              Angle = -2.716780186,  
              Base = [[palette.entities.creatures.chhfd3]],  
              InheritPos = 1,  
              Name = [[Domesticated Mektoub]],  
              PlayerAttackable = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_170]],  
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
                InstanceId = [[Client1_171]],  
                Class = [[Position]],  
                x = 27095.57813,  
                y = -12943.8125,  
                z = -19.09375
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_120]],  
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
        x = 0,  
        y = 0,  
        z = 0
      }
    },  
    {
      InstanceId = [[Client1_891]],  
      Class = [[Act]],  
      Version = 6,  
      InheritPos = 1,  
      LocationId = [[Client1_893]],  
      ManualWeather = 1,  
      Name = [[Act 2]],  
      Season = 0,  
      ShortDescription = [[A way must be found through the Kitin infested Prime Roots to a safe homin camp!]],  
      Title = [[]],  
      WeatherValue = 0,  
      ActivitiesIds = {
      },  
      Behavior = {
        InstanceId = [[Client1_889]],  
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
          InstanceId = [[Client1_892]],  
          Class = [[DefaultFeature]],  
          Components = {
            {
              InstanceId = [[Client1_1006]],  
              Class = [[NpcCustom]],  
              Angle = -1.84375,  
              ArmColor = 4,  
              ArmModel = 6714926,  
              Base = [[palette.entities.npcs.civils.m_civil_220]],  
              BotAttackable = 1,  
              EyesColor = 5,  
              FeetColor = 4,  
              FeetModel = 6715950,  
              GabaritArmsWidth = 8,  
              GabaritBreastSize = 8,  
              GabaritHeight = 0,  
              GabaritLegsWidth = 8,  
              GabaritTorsoWidth = 3,  
              HairColor = 2,  
              HairType = 5622574,  
              HandsColor = 4,  
              HandsModel = 6713902,  
              InheritPos = 1,  
              JacketColor = 4,  
              JacketModel = 6715438,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 3,  
              MorphTarget2 = 5,  
              MorphTarget3 = 2,  
              MorphTarget4 = 5,  
              MorphTarget5 = 5,  
              MorphTarget6 = 6,  
              MorphTarget7 = 3,  
              MorphTarget8 = 7,  
              Name = [[Gine Nidera]],  
              NoRespawn = 1,  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_civil_light_melee_blunt_f2.creature]],  
              SheetClient = [[basic_matis_female.creature]],  
              Speed = 0,  
              Tattoo = 16,  
              TrouserColor = 4,  
              TrouserModel = 6714414,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1004]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_1171]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_1176]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_1147]]),  
                        Action = {
                          InstanceId = [[Client1_1175]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_5795]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_5760]]),  
                        Action = {
                          InstanceId = [[Client1_5794]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_5797]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_1010]]),  
                        Action = {
                          InstanceId = [[Client1_5796]],  
                          Class = [[ActionType]],  
                          Type = [[begin activity sequence]],  
                          Value = r2.RefId([[Client1_1349]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_5799]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_1035]]),  
                        Action = {
                          InstanceId = [[Client1_5798]],  
                          Class = [[ActionType]],  
                          Type = [[begin activity sequence]],  
                          Value = r2.RefId([[Client1_1352]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_5801]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_1031]]),  
                        Action = {
                          InstanceId = [[Client1_5800]],  
                          Class = [[ActionType]],  
                          Type = [[begin activity sequence]],  
                          Value = r2.RefId([[Client1_1227]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_1172]],  
                      Class = [[EventType]],  
                      Type = [[death]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_1346]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                    }
                  },  
                  {
                    InstanceId = [[Client1_1347]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_1348]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_1201]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      },  
                      {
                        InstanceId = [[Client1_1452]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_1407]]),  
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
                    InstanceId = [[Client1_1724]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_1725]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_1680]]),  
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
                InstanceId = [[Client1_1007]],  
                Class = [[Position]],  
                x = 32695.75,  
                y = -21076.79688,  
                z = 7.46875
              }
            },  
            {
              InstanceId = [[Client1_1010]],  
              Class = [[NpcCustom]],  
              Angle = -1.875,  
              ArmColor = 1,  
              ArmModel = 6717230,  
              Base = [[palette.entities.npcs.civils.m_civil_220]],  
              BotAttackable = 1,  
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
              InheritPos = 1,  
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
              Name = [[Trigio Nidera]],  
              NoRespawn = 1,  
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
                InstanceId = [[Client1_1008]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_1180]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_1185]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_1147]]),  
                        Action = {
                          InstanceId = [[Client1_1184]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_5787]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_5760]]),  
                        Action = {
                          InstanceId = [[Client1_5786]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_5789]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_1006]]),  
                        Action = {
                          InstanceId = [[Client1_5788]],  
                          Class = [[ActionType]],  
                          Type = [[begin activity sequence]],  
                          Value = r2.RefId([[Client1_1346]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_5791]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_1031]]),  
                        Action = {
                          InstanceId = [[Client1_5790]],  
                          Class = [[ActionType]],  
                          Type = [[begin activity sequence]],  
                          Value = r2.RefId([[Client1_1227]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_5793]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_1035]]),  
                        Action = {
                          InstanceId = [[Client1_5792]],  
                          Class = [[ActionType]],  
                          Type = [[begin activity sequence]],  
                          Value = r2.RefId([[Client1_1352]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_1181]],  
                      Class = [[EventType]],  
                      Type = [[death]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_1349]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                    }
                  },  
                  {
                    InstanceId = [[Client1_1350]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_1351]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_1201]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      },  
                      {
                        InstanceId = [[Client1_1451]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_1407]]),  
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
                    InstanceId = [[Client1_1726]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_1727]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_1680]]),  
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
                InstanceId = [[Client1_1011]],  
                Class = [[Position]],  
                x = 32694.34375,  
                y = -21076.84375,  
                z = 7.390625
              }
            },  
            {
              InstanceId = [[Client1_1035]],  
              Class = [[NpcCustom]],  
              Angle = 3.078125,  
              ArmColor = 4,  
              ArmModel = 6701614,  
              Base = [[palette.entities.npcs.guards.f_guard_245]],  
              EyesColor = 0,  
              FeetColor = 4,  
              FeetModel = 6699566,  
              GabaritArmsWidth = 7,  
              GabaritBreastSize = 5,  
              GabaritHeight = 1,  
              GabaritLegsWidth = 8,  
              GabaritTorsoWidth = 2,  
              HairColor = 4,  
              HairType = 6700590,  
              HandsColor = 4,  
              HandsModel = 6700078,  
              InheritPos = 1,  
              JacketColor = 4,  
              JacketModel = 6702126,  
              Level = 0,  
              LinkColor = 1,  
              MorphTarget1 = 4,  
              MorphTarget2 = 3,  
              MorphTarget3 = 6,  
              MorphTarget4 = 6,  
              MorphTarget5 = 1,  
              MorphTarget6 = 6,  
              MorphTarget7 = 3,  
              MorphTarget8 = 2,  
              Name = [[Abyrixius Iodix]],  
              NoRespawn = 1,  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_guard_melee_tank_blunt_f4.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 0,  
              Tattoo = 28,  
              TrouserColor = 4,  
              TrouserModel = 6701102,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6755374,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1033]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_1187]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_1192]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_1147]]),  
                        Action = {
                          InstanceId = [[Client1_1191]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_5803]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_5760]]),  
                        Action = {
                          InstanceId = [[Client1_5802]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_5805]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_1006]]),  
                        Action = {
                          InstanceId = [[Client1_5804]],  
                          Class = [[ActionType]],  
                          Type = [[begin activity sequence]],  
                          Value = r2.RefId([[Client1_1346]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_5807]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_1010]]),  
                        Action = {
                          InstanceId = [[Client1_5806]],  
                          Class = [[ActionType]],  
                          Type = [[begin activity sequence]],  
                          Value = r2.RefId([[Client1_1349]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_5809]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_1031]]),  
                        Action = {
                          InstanceId = [[Client1_5808]],  
                          Class = [[ActionType]],  
                          Type = [[begin activity sequence]],  
                          Value = r2.RefId([[Client1_1227]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_1188]],  
                      Class = [[EventType]],  
                      Type = [[death]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_1352]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                    }
                  },  
                  {
                    InstanceId = [[Client1_1353]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_1354]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_1201]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      },  
                      {
                        InstanceId = [[Client1_1449]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_1407]]),  
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
                    InstanceId = [[Client1_1722]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_1723]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_1680]]),  
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
                InstanceId = [[Client1_1036]],  
                Class = [[Position]],  
                x = 32697.5,  
                y = -21083.67188,  
                z = 8.34375
              }
            },  
            {
              InstanceId = [[Client1_1031]],  
              Class = [[NpcCustom]],  
              Angle = 3.078125,  
              ArmColor = 4,  
              ArmModel = 6722862,  
              Base = [[palette.entities.npcs.guards.t_guard_245]],  
              EyesColor = 3,  
              FeetColor = 4,  
              FeetModel = 6720814,  
              GabaritArmsWidth = 6,  
              GabaritBreastSize = 6,  
              GabaritHeight = 6,  
              GabaritLegsWidth = 8,  
              GabaritTorsoWidth = 4,  
              HairColor = 4,  
              HairType = 6721838,  
              HandsColor = 4,  
              HandsModel = 6721326,  
              InheritPos = 1,  
              JacketColor = 4,  
              JacketModel = 6723374,  
              Level = 0,  
              LinkColor = 1,  
              MorphTarget1 = 3,  
              MorphTarget2 = 2,  
              MorphTarget3 = 3,  
              MorphTarget4 = 2,  
              MorphTarget5 = 4,  
              MorphTarget6 = 3,  
              MorphTarget7 = 0,  
              MorphTarget8 = 3,  
              Name = [[Gale Brapie]],  
              NoRespawn = 1,  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_guard_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_tryker_female.creature]],  
              Speed = 0,  
              Tattoo = 16,  
              TrouserColor = 4,  
              TrouserModel = 6722350,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6766894,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1029]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_1194]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_1199]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_1147]]),  
                        Action = {
                          InstanceId = [[Client1_1198]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_5811]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_5760]]),  
                        Action = {
                          InstanceId = [[Client1_5810]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_5813]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_1006]]),  
                        Action = {
                          InstanceId = [[Client1_5812]],  
                          Class = [[ActionType]],  
                          Type = [[begin activity sequence]],  
                          Value = r2.RefId([[Client1_1346]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_5815]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_1010]]),  
                        Action = {
                          InstanceId = [[Client1_5814]],  
                          Class = [[ActionType]],  
                          Type = [[begin activity sequence]],  
                          Value = r2.RefId([[Client1_1349]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_5817]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_1035]]),  
                        Action = {
                          InstanceId = [[Client1_5816]],  
                          Class = [[ActionType]],  
                          Type = [[begin activity sequence]],  
                          Value = r2.RefId([[Client1_1352]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_1195]],  
                      Class = [[EventType]],  
                      Type = [[death]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5727]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_5730]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_2151]]),  
                        Action = {
                          InstanceId = [[Client1_5729]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_5728]],  
                      Class = [[EventType]],  
                      Type = [[end of activity sequence]],  
                      Value = r2.RefId([[Client1_1228]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5732]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_5735]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_2151]]),  
                        Action = {
                          InstanceId = [[Client1_5734]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_5733]],  
                      Class = [[EventType]],  
                      Type = [[end of activity sequence]],  
                      Value = r2.RefId([[Client1_1720]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_1227]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                    }
                  },  
                  {
                    InstanceId = [[Client1_1228]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_1229]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_1201]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      },  
                      {
                        InstanceId = [[Client1_1450]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_1407]]),  
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
                    InstanceId = [[Client1_1720]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_1721]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_1680]]),  
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
                InstanceId = [[Client1_1032]],  
                Class = [[Position]],  
                x = 32698.39063,  
                y = -21081.65625,  
                z = 8.25
              }
            },  
            {
              InstanceId = [[Client1_1066]],  
              Class = [[Npc]],  
              Angle = 0.875,  
              Base = [[palette.entities.botobjects.wind_turbine]],  
              InheritPos = 1,  
              Name = [[wind turbine 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1064]],  
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
                InstanceId = [[Client1_1067]],  
                Class = [[Position]],  
                x = 32260.96875,  
                y = -21495.96875,  
                z = -23.578125
              }
            },  
            {
              InstanceId = [[Client1_1977]],  
              Class = [[NpcCreature]],  
              Angle = -0.7919484973,  
              Base = [[palette.entities.creatures.ckbif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kinrey]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1975]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_1987]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_1988]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Patrol]],  
                        ActivityZoneId = r2.RefId([[Client1_1980]]),  
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
                InstanceId = [[Client1_1978]],  
                Class = [[Position]],  
                x = 32520.90625,  
                y = -21186.375,  
                z = -14.90625
              }
            },  
            {
              InstanceId = [[Client1_2108]],  
              Class = [[NpcCreature]],  
              Angle = -0.2143515646,  
              Base = [[palette.entities.creatures.ckeif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kipucker]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2106]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_2110]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_2111]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Guard Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_2007]]),  
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
                InstanceId = [[Client1_2109]],  
                Class = [[Position]],  
                x = 32336.03125,  
                y = -21166.70313,  
                z = -14.859375
              }
            },  
            {
              InstanceId = [[Client1_2147]],  
              Class = [[NpcCustom]],  
              Angle = 0.90625,  
              ArmColor = 0,  
              ArmModel = 6733614,  
              Base = [[palette.entities.npcs.guards.z_guard_245]],  
              EyesColor = 2,  
              FeetColor = 0,  
              FeetModel = 6731566,  
              GabaritArmsWidth = 9,  
              GabaritBreastSize = 12,  
              GabaritHeight = 14,  
              GabaritLegsWidth = 6,  
              GabaritTorsoWidth = 7,  
              HairColor = 0,  
              HairType = 6732590,  
              HandsColor = 0,  
              HandsModel = 6732078,  
              InheritPos = 1,  
              JacketColor = 0,  
              JacketModel = 6734126,  
              Level = 0,  
              LinkColor = 1,  
              MorphTarget1 = 3,  
              MorphTarget2 = 1,  
              MorphTarget3 = 0,  
              MorphTarget4 = 1,  
              MorphTarget5 = 6,  
              MorphTarget6 = 5,  
              MorphTarget7 = 5,  
              MorphTarget8 = 1,  
              Name = [[Be-ci Kuani]],  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_guard_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_zorai_male.creature]],  
              Speed = 0,  
              Tattoo = 3,  
              TrouserColor = 0,  
              TrouserModel = 6733102,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6771758,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2145]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_2517]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_2520]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_2506]]),  
                        Action = {
                          InstanceId = [[Client1_2519]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_2522]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_2488]]),  
                        Action = {
                          InstanceId = [[Client1_2521]],  
                          Class = [[ActionType]],  
                          Type = [[Activate]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_2518]],  
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
                InstanceId = [[Client1_2148]],  
                Class = [[Position]],  
                x = 32266.29688,  
                y = -21476.01563,  
                z = -20.609375
              }
            }
          }
        },  
        {
          InstanceId = [[Client1_1022]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 1,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Start]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_1020]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_1023]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 2,  
              Actions = {
                {
                  InstanceId = [[Client1_1024]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_1027]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            },  
            {
              InstanceId = [[Client1_1676]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_1677]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_1678]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            },  
            {
              InstanceId = [[Client1_1025]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_1026]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_1028]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_1021]],  
            Class = [[Position]],  
            x = 32690.17188,  
            y = -21068.79688,  
            z = 5
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_1040]],  
          Class = [[TalkTo]],  
          Active = 1,  
          Base = [[palette.entities.botobjects.bot_chat]],  
          ContextualText = [[Accept <mission_target>'s plan]],  
          InheritPos = 1,  
          MissionGiver = r2.RefId([[Client1_1010]]),  
          MissionSucceedText = [[I am glad you agree with me, let us move out!]],  
          MissionTarget = r2.RefId([[Client1_1031]]),  
          MissionText = [[Finally we have reached the Prime Roots.  There is a camp we must get to in the South West, but Kitin have overrun the area!  Gale Brapie and Abyrixius Iodix here have some plans on how to get there, choose the one you think best.]],  
          Name = [[Mission: Talk to Gale]],  
          Repeatable = 0,  
          WaitValidationText = [[Hello there!  You wish to hear my plan?  Well there are two routes, one takes us into an open area with room to fight, the other is a confined corridor.  I think we should head West then South to the open area!]],  
          _Seed = 1154302056,  
          Behavior = {
            InstanceId = [[Client1_1041]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_1052]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_1055]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_1043]]),  
                    Action = {
                      InstanceId = [[Client1_1054]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_1053]],  
                  Class = [[EventType]],  
                  Type = [[succeeded]],  
                  Value = r2.RefId([[]])
                }
              },  
              {
                InstanceId = [[Client1_1231]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_1234]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_1031]]),  
                    Action = {
                      InstanceId = [[Client1_1233]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_1228]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_1356]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_1010]]),  
                    Action = {
                      InstanceId = [[Client1_1355]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_1350]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_1358]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_1006]]),  
                    Action = {
                      InstanceId = [[Client1_1357]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_1347]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_1360]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_1035]]),  
                    Action = {
                      InstanceId = [[Client1_1359]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_1353]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_1232]],  
                  Class = [[EventType]],  
                  Type = [[succeeded]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
          },  
          Position = {
            InstanceId = [[Client1_1042]],  
            Class = [[Position]],  
            x = 32689.79688,  
            y = -21072.54688,  
            z = 7
          }
        },  
        {
          InstanceId = [[Client1_1043]],  
          Class = [[TalkTo]],  
          Active = 1,  
          Base = [[palette.entities.botobjects.bot_chat]],  
          ContextualText = [[Accept <mission_target>'s plan]],  
          InheritPos = 1,  
          MissionGiver = r2.RefId([[Client1_1010]]),  
          MissionSucceedText = [[You have made a wise decision, let us move out!]],  
          MissionTarget = r2.RefId([[Client1_1035]]),  
          MissionText = [[]],  
          Name = [[Mission: Talk to Aby]],  
          Repeatable = 0,  
          WaitValidationText = [[Hello there.  You wish to hear my plan I guess?  Well I think we should head South as there are less Kitin on that route, also it is a more confined space so less chance of them outmanuvering us.]],  
          _Seed = 1154302386,  
          Behavior = {
            InstanceId = [[Client1_1044]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_1047]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_1050]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_1040]]),  
                    Action = {
                      InstanceId = [[Client1_1049]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_1048]],  
                  Class = [[EventType]],  
                  Type = [[succeeded]],  
                  Value = r2.RefId([[]])
                }
              },  
              {
                InstanceId = [[Client1_1729]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_1732]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_1006]]),  
                    Action = {
                      InstanceId = [[Client1_1731]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_1724]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_1734]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_1010]]),  
                    Action = {
                      InstanceId = [[Client1_1733]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_1726]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_1736]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_1035]]),  
                    Action = {
                      InstanceId = [[Client1_1735]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_1722]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_1738]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_1031]]),  
                    Action = {
                      InstanceId = [[Client1_1737]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_1720]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_1730]],  
                  Class = [[EventType]],  
                  Type = [[succeeded]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
          },  
          Position = {
            InstanceId = [[Client1_1045]],  
            Class = [[Position]],  
            x = 32688.8125,  
            y = -21073.84375,  
            z = 7
          }
        },  
        {
          InstanceId = [[Client1_1147]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Mission Fail]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_1145]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_1148]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_1149]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_5784]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            },  
            {
              InstanceId = [[Client1_1150]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 4,  
              Actions = {
                {
                  InstanceId = [[Client1_1151]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_5785]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_1146]],  
            Class = [[Position]],  
            x = 32687.70313,  
            y = -21066.67188,  
            z = 5
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_1755]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Group 6]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_1753]],  
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
              InstanceId = [[Client1_1741]],  
              Class = [[NpcCreature]],  
              Angle = -0.317751497,  
              Base = [[palette.entities.creatures.ckfif1]],  
              InheritPos = 1,  
              Name = [[Overlord Kirosta]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1739]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_1796]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_1797]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Patrol]],  
                        ActivityZoneId = r2.RefId([[Client1_1787]]),  
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
                InstanceId = [[Client1_1742]],  
                Class = [[Position]],  
                x = 32598.26563,  
                y = -21180.75,  
                z = 6.375
              }
            },  
            {
              InstanceId = [[Client1_1750]],  
              Class = [[NpcCreature]],  
              Angle = -0.317751497,  
              Base = [[palette.entities.creatures.ckfif1]],  
              InheritPos = 1,  
              Name = [[Overlord Kirosta]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1751]],  
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
                InstanceId = [[Client1_1752]],  
                Class = [[Position]],  
                x = 32599.34375,  
                y = -21178.28125,  
                z = 6.375
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_1754]],  
            Class = [[Position]],  
            x = 0,  
            y = 0,  
            z = 0
          }
        },  
        {
          InstanceId = [[Client1_1773]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Group 7]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_1785]],  
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
              InstanceId = [[Client1_1777]],  
              Class = [[NpcCreature]],  
              Angle = -0.317751497,  
              Base = [[palette.entities.creatures.ckfif1]],  
              InheritPos = 1,  
              Name = [[Overlord Kirosta]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1778]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_1794]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_1795]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Patrol]],  
                        ActivityZoneId = r2.RefId([[Client1_1787]]),  
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
                InstanceId = [[Client1_1779]],  
                Class = [[Position]],  
                x = 32598.26563,  
                y = -21180.75,  
                z = 6.375
              }
            },  
            {
              InstanceId = [[Client1_1782]],  
              Class = [[NpcCreature]],  
              Angle = -0.317751497,  
              Base = [[palette.entities.creatures.ckfif1]],  
              InheritPos = 1,  
              Name = [[Overlord Kirosta]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1783]],  
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
                InstanceId = [[Client1_1784]],  
                Class = [[Position]],  
                x = 32599.34375,  
                y = -21178.28125,  
                z = 6.375
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_1774]],  
            Class = [[Position]],  
            x = 60.25,  
            y = -28.84375,  
            z = -6.359375
          }
        },  
        {
          InstanceId = [[Client1_1814]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Group 8]],  
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
          Components = {
            {
              InstanceId = [[Client1_1800]],  
              Class = [[NpcCreature]],  
              Angle = 2.607978344,  
              Base = [[palette.entities.creatures.ckbif2]],  
              InheritPos = 1,  
              Name = [[Power Overlord Kinrey]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1798]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_1829]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_1830]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Guard Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_1816]]),  
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
                InstanceId = [[Client1_1801]],  
                Class = [[Position]],  
                x = 32677.10938,  
                y = -21315.01563,  
                z = 5.484375
              }
            },  
            {
              InstanceId = [[Client1_1809]],  
              Class = [[NpcCreature]],  
              Angle = 2.607978344,  
              Base = [[palette.entities.creatures.ckbif2]],  
              InheritPos = 1,  
              Name = [[Power Overlord Kinrey]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1810]],  
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
                x = 32679.3125,  
                y = -21306.35938,  
                z = 5.734375
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_1813]],  
            Class = [[Position]],  
            x = 0,  
            y = 0,  
            z = 0
          }
        },  
        {
          InstanceId = [[Client1_1847]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Group 9]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_1845]],  
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
              InstanceId = [[Client1_1833]],  
              Class = [[NpcCreature]],  
              Angle = 1.672338843,  
              Base = [[palette.entities.creatures.ckdif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kincher]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1831]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_1867]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_1868]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Patrol]],  
                        ActivityZoneId = r2.RefId([[Client1_1860]]),  
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
                InstanceId = [[Client1_1834]],  
                Class = [[Position]],  
                x = 32605.85938,  
                y = -21452.46875,  
                z = -8.6875
              }
            },  
            {
              InstanceId = [[Client1_1842]],  
              Class = [[NpcCreature]],  
              Angle = 1.672338843,  
              Base = [[palette.entities.creatures.ckdif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kincher]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1843]],  
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
                InstanceId = [[Client1_1844]],  
                Class = [[Position]],  
                x = 32612.20313,  
                y = -21452.21875,  
                z = -7.71875
              }
            },  
            {
              InstanceId = [[Client1_1855]],  
              Class = [[NpcCreature]],  
              Angle = 1.672338843,  
              Base = [[palette.entities.creatures.ckdif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kincher]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1856]],  
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
                InstanceId = [[Client1_1858]],  
                Class = [[Position]],  
                x = 32597.42188,  
                y = -21457.07813,  
                z = -9.828125
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_1846]],  
            Class = [[Position]],  
            x = 0,  
            y = 0,  
            z = 0
          }
        },  
        {
          InstanceId = [[Client1_1925]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Group 10]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_1923]],  
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
              InstanceId = [[Client1_1900]],  
              Class = [[NpcCreature]],  
              Angle = -0.04343206063,  
              Base = [[palette.entities.creatures.ckaif4]],  
              InheritPos = 1,  
              Name = [[Elite Overlord Kidinak]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1901]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_1940]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_1941]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Guard Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_1927]]),  
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
                InstanceId = [[Client1_1902]],  
                Class = [[Position]],  
                x = 32520.65625,  
                y = -21460.25,  
                z = -19.953125
              }
            },  
            {
              InstanceId = [[Client1_1910]],  
              Class = [[NpcCreature]],  
              Angle = -0.04343206063,  
              Base = [[palette.entities.creatures.ckaif4]],  
              InheritPos = 1,  
              Name = [[Elite Overlord Kidinak]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1911]],  
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
                InstanceId = [[Client1_1912]],  
                Class = [[Position]],  
                x = 32520.75,  
                y = -21456.34375,  
                z = -20.5
              }
            },  
            {
              InstanceId = [[Client1_1920]],  
              Class = [[NpcCreature]],  
              Angle = -0.04343206063,  
              Base = [[palette.entities.creatures.ckaif4]],  
              InheritPos = 1,  
              Name = [[Elite Overlord Kidinak]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1921]],  
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
                InstanceId = [[Client1_1922]],  
                Class = [[Position]],  
                x = 32514.82813,  
                y = -21456,  
                z = -20.515625
              }
            },  
            {
              InstanceId = [[Client1_1890]],  
              Class = [[NpcCreature]],  
              Angle = -0.04343206063,  
              Base = [[palette.entities.creatures.ckaif4]],  
              InheritPos = 1,  
              Name = [[Elite Overlord Kidinak]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1891]],  
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
                InstanceId = [[Client1_1892]],  
                Class = [[Position]],  
                x = 32520.34375,  
                y = -21464.5,  
                z = -19.53125
              }
            },  
            {
              InstanceId = [[Client1_1871]],  
              Class = [[NpcCreature]],  
              Angle = -0.04343206063,  
              Base = [[palette.entities.creatures.ckaif4]],  
              InheritPos = 1,  
              Name = [[Elite Overlord Kidinak]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1869]],  
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
                InstanceId = [[Client1_1872]],  
                Class = [[Position]],  
                x = 32516.25,  
                y = -21461.67188,  
                z = -19.734375
              }
            },  
            {
              InstanceId = [[Client1_1880]],  
              Class = [[NpcCreature]],  
              Angle = -0.04343206063,  
              Base = [[palette.entities.creatures.ckaif4]],  
              InheritPos = 1,  
              Name = [[Elite Overlord Kidinak]],  
              NoRespawn = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1881]],  
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
                InstanceId = [[Client1_1882]],  
                Class = [[Position]],  
                x = 32516.5625,  
                y = -21466,  
                z = -19.109375
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_1924]],  
            Class = [[Position]],  
            x = 0,  
            y = 0,  
            z = 0
          }
        },  
        {
          InstanceId = [[Client1_1958]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Group 11]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_1956]],  
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
              InstanceId = [[Client1_1944]],  
              Class = [[NpcCreature]],  
              Angle = -0.3125315011,  
              Base = [[palette.entities.creatures.ckgif4]],  
              InheritPos = 1,  
              Name = [[Elite Overlord Kiban]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1942]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_1973]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_1974]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Guard Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_1960]]),  
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
                InstanceId = [[Client1_1945]],  
                Class = [[Position]],  
                x = 32388.60938,  
                y = -21517.76563,  
                z = -24.65625
              }
            },  
            {
              InstanceId = [[Client1_1953]],  
              Class = [[NpcCreature]],  
              Angle = -0.3125315011,  
              Base = [[palette.entities.creatures.ckgif4]],  
              InheritPos = 1,  
              Name = [[Elite Overlord Kiban]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1954]],  
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
                InstanceId = [[Client1_1955]],  
                Class = [[Position]],  
                x = 32401.09375,  
                y = -21519.8125,  
                z = -24.65625
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_1957]],  
            Class = [[Position]],  
            x = -0.25,  
            y = -12.46875,  
            z = 2.203125
          }
        },  
        {
          InstanceId = [[Client1_2005]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Group 12]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_2003]],  
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
              InstanceId = [[Client1_1991]],  
              Class = [[NpcCreature]],  
              Angle = -0.8612123728,  
              Base = [[palette.entities.creatures.ckfrf4]],  
              InheritPos = 1,  
              Name = [[Elite Commander Kirosta]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_1989]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_2026]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_2027]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Guard Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_2007]]),  
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
                InstanceId = [[Client1_1992]],  
                Class = [[Position]],  
                x = 32459.54688,  
                y = -21201.53125,  
                z = -21.5
              }
            },  
            {
              InstanceId = [[Client1_2000]],  
              Class = [[NpcCreature]],  
              Angle = -0.8612123728,  
              Base = [[palette.entities.creatures.ckfrf4]],  
              InheritPos = 1,  
              Name = [[Elite Commander Kirosta]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2001]],  
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
                InstanceId = [[Client1_2002]],  
                Class = [[Position]],  
                x = 32454.09375,  
                y = -21206.5,  
                z = -21.140625
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_2004]],  
            Class = [[Position]],  
            x = 0,  
            y = 0,  
            z = 0
          }
        },  
        {
          InstanceId = [[Client1_2065]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Group 13]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_2063]],  
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
              InstanceId = [[Client1_2030]],  
              Class = [[NpcCreature]],  
              Angle = 0.04004672915,  
              Base = [[palette.entities.creatures.ckfif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kirosta]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2028]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_2066]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_2067]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Guard Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_2007]]),  
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
                InstanceId = [[Client1_2031]],  
                Class = [[Position]],  
                x = 32361.14063,  
                y = -21166.95313,  
                z = -18.546875
              }
            },  
            {
              InstanceId = [[Client1_2060]],  
              Class = [[NpcCreature]],  
              Angle = 0.04004672915,  
              Base = [[palette.entities.creatures.ckfif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kirosta]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2061]],  
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
                InstanceId = [[Client1_2062]],  
                Class = [[Position]],  
                x = 32361.85938,  
                y = -21163.10938,  
                z = -18.9375
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_2064]],  
            Class = [[Position]],  
            x = 0,  
            y = 0,  
            z = 0
          }
        },  
        {
          InstanceId = [[Client1_2084]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Group 14]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_2082]],  
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
              InstanceId = [[Client1_2070]],  
              Class = [[NpcCreature]],  
              Angle = -1.370728374,  
              Base = [[palette.entities.creatures.ckbif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kinrey]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2068]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_2085]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_2086]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Guard Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_2007]]),  
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
                InstanceId = [[Client1_2071]],  
                Class = [[Position]],  
                x = 32395.21875,  
                y = -21138.15625,  
                z = -13.4375
              }
            },  
            {
              InstanceId = [[Client1_2079]],  
              Class = [[NpcCreature]],  
              Angle = -1.370728374,  
              Base = [[palette.entities.creatures.ckbif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kinrey]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2080]],  
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
                InstanceId = [[Client1_2081]],  
                Class = [[Position]],  
                x = 32398.35938,  
                y = -21138.48438,  
                z = -12.890625
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_2083]],  
            Class = [[Position]],  
            x = 0,  
            y = 0,  
            z = 0
          }
        },  
        {
          InstanceId = [[Client1_2103]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Group 15]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_2101]],  
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
              InstanceId = [[Client1_2089]],  
              Class = [[NpcCreature]],  
              Angle = -0.5690534115,  
              Base = [[palette.entities.creatures.ckaif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kidinak]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2087]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_2104]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_2105]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Guard Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_2007]]),  
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
                InstanceId = [[Client1_2090]],  
                Class = [[Position]],  
                x = 32372.46875,  
                y = -21146.53125,  
                z = -18.671875
              }
            },  
            {
              InstanceId = [[Client1_2098]],  
              Class = [[NpcCreature]],  
              Angle = -0.5690534115,  
              Base = [[palette.entities.creatures.ckaif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kidinak]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2099]],  
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
                InstanceId = [[Client1_2100]],  
                Class = [[Position]],  
                x = 32374.29688,  
                y = -21144.14063,  
                z = -18.125
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_2102]],  
            Class = [[Position]],  
            x = 0,  
            y = 0,  
            z = 0
          }
        },  
        {
          InstanceId = [[Client1_2128]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Group 16]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_2126]],  
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
              InstanceId = [[Client1_2114]],  
              Class = [[NpcCreature]],  
              Angle = 2.106621742,  
              Base = [[palette.entities.creatures.ckjif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kipesta]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2112]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_2143]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_2144]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Guard Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_2130]]),  
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
                InstanceId = [[Client1_2115]],  
                Class = [[Position]],  
                x = 32334.78125,  
                y = -21300.85938,  
                z = 1.25
              }
            },  
            {
              InstanceId = [[Client1_2123]],  
              Class = [[NpcCreature]],  
              Angle = 2.106621742,  
              Base = [[palette.entities.creatures.ckjif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kipesta]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2124]],  
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
                InstanceId = [[Client1_2125]],  
                Class = [[Position]],  
                x = 32332.28125,  
                y = -21302.39063,  
                z = 0.5
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_2127]],  
            Class = [[Position]],  
            x = 0,  
            y = 0,  
            z = 0
          }
        },  
        {
          InstanceId = [[Client1_2151]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Enter camp]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_2149]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_5743]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_5746]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2506]]),  
                    Action = {
                      InstanceId = [[Client1_5745]],  
                      Class = [[ActionType]],  
                      Type = [[activate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_5748]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2151]]),  
                    Action = {
                      InstanceId = [[Client1_5747]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_5744]],  
                  Class = [[EventType]],  
                  Type = [[end of dialog]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_2152]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_2153]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_2156]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            },  
            {
              InstanceId = [[Client1_2154]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_2155]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_2157]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_2150]],  
            Class = [[Position]],  
            x = 32262.90625,  
            y = -21474.04688,  
            z = -21
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_2175]],  
          Class = [[ZoneTrigger]],  
          Active = 1,  
          Base = [[palette.entities.botobjects.trigger_zone]],  
          Cyclic = 0,  
          InheritPos = 1,  
          Name = [[Zone trigger 1]],  
          _Zone = [[Client1_2179]],  
          Behavior = {
            InstanceId = [[Client1_2176]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_2179]],  
              Class = [[Region]],  
              Deletable = 0,  
              InheritPos = 1,  
              Name = [[Camp]],  
              Points = {
                {
                  InstanceId = [[Client1_2181]],  
                  Class = [[RegionVertex]],  
                  Deletable = 0,  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_2182]],  
                    Class = [[Position]],  
                    x = 43.75,  
                    y = 15.296875,  
                    z = -1.734375
                  }
                },  
                {
                  InstanceId = [[Client1_2184]],  
                  Class = [[RegionVertex]],  
                  Deletable = 0,  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_2185]],  
                    Class = [[Position]],  
                    x = -11.578125,  
                    y = 55.484375,  
                    z = -0.28125
                  }
                },  
                {
                  InstanceId = [[Client1_2187]],  
                  Class = [[RegionVertex]],  
                  Deletable = 0,  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_2188]],  
                    Class = [[Position]],  
                    x = -41.0625,  
                    y = -14.015625,  
                    z = -0.921875
                  }
                },  
                {
                  InstanceId = [[Client1_2190]],  
                  Class = [[RegionVertex]],  
                  Deletable = 0,  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_2191]],  
                    Class = [[Position]],  
                    x = 9.078125,  
                    y = -41.53125,  
                    z = -1.9375
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_2178]],  
                Class = [[Position]],  
                x = -16.84375,  
                y = 22.203125,  
                z = 2.359375
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_2177]],  
            Class = [[Position]],  
            x = 32292.20313,  
            y = -21497.79688,  
            z = -23
          }
        },  
        {
          InstanceId = [[Client1_2207]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Group 17]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_2205]],  
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
              InstanceId = [[Client1_2199]],  
              Class = [[NpcCustom]],  
              Angle = -1.25,  
              ArmColor = 3,  
              ArmModel = 6701614,  
              Base = [[palette.entities.npcs.guards.f_guard_245]],  
              EyesColor = 3,  
              FeetColor = 3,  
              FeetModel = 6699566,  
              GabaritArmsWidth = 2,  
              GabaritBreastSize = 11,  
              GabaritHeight = 4,  
              GabaritLegsWidth = 3,  
              GabaritTorsoWidth = 2,  
              HairColor = 3,  
              HairType = 6700590,  
              HandsColor = 3,  
              HandsModel = 6700078,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6702126,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 3,  
              MorphTarget2 = 2,  
              MorphTarget3 = 4,  
              MorphTarget4 = 2,  
              MorphTarget5 = 3,  
              MorphTarget6 = 6,  
              MorphTarget7 = 0,  
              MorphTarget8 = 6,  
              Name = [[Kriion]],  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_guard_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 0,  
              Tattoo = 26,  
              TrouserColor = 3,  
              TrouserModel = 6701102,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6756910,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2197]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_2337]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_2338]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Wander]],  
                        ActivityZoneId = r2.RefId([[Client1_2179]]),  
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
                InstanceId = [[Client1_2200]],  
                Class = [[Position]],  
                x = 32288.60938,  
                y = -21435.71875,  
                z = -21.265625
              }
            },  
            {
              InstanceId = [[Client1_2203]],  
              Class = [[NpcCustom]],  
              Angle = -0.9375,  
              ArmColor = 3,  
              ArmModel = 6701614,  
              Base = [[palette.entities.npcs.guards.f_guard_245]],  
              EyesColor = 6,  
              FeetColor = 3,  
              FeetModel = 6699566,  
              GabaritArmsWidth = 3,  
              GabaritBreastSize = 4,  
              GabaritHeight = 3,  
              GabaritLegsWidth = 2,  
              GabaritTorsoWidth = 0,  
              HairColor = 3,  
              HairType = 6700590,  
              HandsColor = 3,  
              HandsModel = 6700078,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6702126,  
              Level = 0,  
              LinkColor = 1,  
              MorphTarget1 = 4,  
              MorphTarget2 = 1,  
              MorphTarget3 = 2,  
              MorphTarget4 = 0,  
              MorphTarget5 = 4,  
              MorphTarget6 = 1,  
              MorphTarget7 = 1,  
              MorphTarget8 = 5,  
              Name = [[Iolaus]],  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_guard_melee_tank_pierce_f4.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 0,  
              Tattoo = 9,  
              TrouserColor = 3,  
              TrouserModel = 6701102,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6755630,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2201]],  
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
                InstanceId = [[Client1_2204]],  
                Class = [[Position]],  
                x = 32289.5,  
                y = -21434.90625,  
                z = -21.265625
              }
            },  
            {
              InstanceId = [[Client1_2210]],  
              Class = [[NpcCustom]],  
              Angle = -0.9375,  
              ArmColor = 3,  
              ArmModel = 6701614,  
              Base = [[palette.entities.npcs.guards.f_guard_245]],  
              EyesColor = 5,  
              FeetColor = 3,  
              FeetModel = 6699566,  
              GabaritArmsWidth = 1,  
              GabaritBreastSize = 11,  
              GabaritHeight = 8,  
              GabaritLegsWidth = 3,  
              GabaritTorsoWidth = 5,  
              HairColor = 3,  
              HairType = 6700590,  
              HandsColor = 3,  
              HandsModel = 6700078,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6702126,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 0,  
              MorphTarget2 = 4,  
              MorphTarget3 = 7,  
              MorphTarget4 = 1,  
              MorphTarget5 = 4,  
              MorphTarget6 = 0,  
              MorphTarget7 = 4,  
              MorphTarget8 = 7,  
              Name = [[Aeseus]],  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_guard_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 0,  
              Tattoo = 14,  
              TrouserColor = 3,  
              TrouserModel = 6701102,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6756398,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2208]],  
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
                InstanceId = [[Client1_2212]],  
                Class = [[Position]],  
                x = 32287.6875,  
                y = -21437.03125,  
                z = -21.28125
              }
            },  
            {
              InstanceId = [[Client1_2215]],  
              Class = [[NpcCustom]],  
              Angle = -0.9375,  
              ArmColor = 3,  
              ArmModel = 6701614,  
              Base = [[palette.entities.npcs.guards.f_guard_245]],  
              EyesColor = 6,  
              FeetColor = 3,  
              FeetModel = 6699566,  
              GabaritArmsWidth = 5,  
              GabaritBreastSize = 12,  
              GabaritHeight = 8,  
              GabaritLegsWidth = 9,  
              GabaritTorsoWidth = 1,  
              HairColor = 3,  
              HairType = 6700590,  
              HandsColor = 3,  
              HandsModel = 6700078,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6702126,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 2,  
              MorphTarget2 = 3,  
              MorphTarget3 = 4,  
              MorphTarget4 = 6,  
              MorphTarget5 = 1,  
              MorphTarget6 = 6,  
              MorphTarget7 = 1,  
              MorphTarget8 = 0,  
              Name = [[Aekos]],  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_guard_melee_tank_pierce_f4.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 0,  
              Tattoo = 31,  
              TrouserColor = 3,  
              TrouserModel = 6701102,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6755886,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2213]],  
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
                InstanceId = [[Client1_2217]],  
                Class = [[Position]],  
                x = 32287.15625,  
                y = -21435.10938,  
                z = -21.453125
              }
            },  
            {
              InstanceId = [[Client1_2220]],  
              Class = [[NpcCustom]],  
              Angle = -0.9375,  
              ArmColor = 3,  
              ArmModel = 6701614,  
              Base = [[palette.entities.npcs.guards.f_guard_245]],  
              EyesColor = 4,  
              FeetColor = 3,  
              FeetModel = 6699566,  
              GabaritArmsWidth = 11,  
              GabaritBreastSize = 5,  
              GabaritHeight = 3,  
              GabaritLegsWidth = 4,  
              GabaritTorsoWidth = 4,  
              HairColor = 3,  
              HairType = 6700590,  
              HandsColor = 3,  
              HandsModel = 6700078,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6702126,  
              Level = 0,  
              LinkColor = 1,  
              MorphTarget1 = 6,  
              MorphTarget2 = 5,  
              MorphTarget3 = 5,  
              MorphTarget4 = 5,  
              MorphTarget5 = 0,  
              MorphTarget6 = 3,  
              MorphTarget7 = 3,  
              MorphTarget8 = 1,  
              Name = [[Xarus]],  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_guard_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 0,  
              Tattoo = 11,  
              TrouserColor = 3,  
              TrouserModel = 6701102,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6756910,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2218]],  
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
                InstanceId = [[Client1_2222]],  
                Class = [[Position]],  
                x = 32288.25,  
                y = -21433.79688,  
                z = -21.484375
              }
            },  
            {
              InstanceId = [[Client1_2225]],  
              Class = [[NpcCustom]],  
              Angle = 2.84375,  
              ArmColor = 2,  
              ArmModel = 6733870,  
              Base = [[palette.entities.npcs.guards.z_guard_245]],  
              EyesColor = 4,  
              FeetColor = 2,  
              FeetModel = 6731822,  
              GabaritArmsWidth = 9,  
              GabaritBreastSize = 1,  
              GabaritHeight = 14,  
              GabaritLegsWidth = 0,  
              GabaritTorsoWidth = 2,  
              HairColor = 2,  
              HairType = 6732846,  
              HandsColor = 2,  
              HandsModel = 6732334,  
              InheritPos = 1,  
              JacketColor = 2,  
              JacketModel = 6734382,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 5,  
              MorphTarget2 = 7,  
              MorphTarget3 = 1,  
              MorphTarget4 = 7,  
              MorphTarget5 = 1,  
              MorphTarget6 = 3,  
              MorphTarget7 = 0,  
              MorphTarget8 = 7,  
              Name = [[Fuo]],  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_guard_melee_tank_blunt_f4.creature]],  
              SheetClient = [[basic_zorai_male.creature]],  
              Speed = 0,  
              Tattoo = 12,  
              TrouserColor = 2,  
              TrouserModel = 6733358,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6770734,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2223]],  
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
                InstanceId = [[Client1_2227]],  
                Class = [[Position]],  
                x = 32289.82813,  
                y = -21437.96875,  
                z = -21.09375
              }
            },  
            {
              InstanceId = [[Client1_2230]],  
              Class = [[NpcCustom]],  
              Angle = 2.84375,  
              ArmColor = 2,  
              ArmModel = 6733870,  
              Base = [[palette.entities.npcs.guards.z_guard_245]],  
              EyesColor = 3,  
              FeetColor = 2,  
              FeetModel = 6731822,  
              GabaritArmsWidth = 10,  
              GabaritBreastSize = 9,  
              GabaritHeight = 1,  
              GabaritLegsWidth = 3,  
              GabaritTorsoWidth = 0,  
              HairColor = 2,  
              HairType = 6732846,  
              HandsColor = 2,  
              HandsModel = 6732334,  
              InheritPos = 1,  
              JacketColor = 2,  
              JacketModel = 6734382,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 6,  
              MorphTarget2 = 2,  
              MorphTarget3 = 2,  
              MorphTarget4 = 4,  
              MorphTarget5 = 1,  
              MorphTarget6 = 0,  
              MorphTarget7 = 6,  
              MorphTarget8 = 2,  
              Name = [[Ce-Ni]],  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_guard_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_zorai_male.creature]],  
              Speed = 0,  
              Tattoo = 28,  
              TrouserColor = 2,  
              TrouserModel = 6733358,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6772014,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2228]],  
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
                InstanceId = [[Client1_2232]],  
                Class = [[Position]],  
                x = 32291.21875,  
                y = -21436.40625,  
                z = -21.09375
              }
            },  
            {
              InstanceId = [[Client1_2235]],  
              Class = [[NpcCustom]],  
              Angle = 2.84375,  
              ArmColor = 2,  
              ArmModel = 6733870,  
              Base = [[palette.entities.npcs.guards.z_guard_245]],  
              EyesColor = 1,  
              FeetColor = 2,  
              FeetModel = 6731822,  
              GabaritArmsWidth = 12,  
              GabaritBreastSize = 1,  
              GabaritHeight = 3,  
              GabaritLegsWidth = 7,  
              GabaritTorsoWidth = 1,  
              HairColor = 2,  
              HairType = 6732846,  
              HandsColor = 2,  
              HandsModel = 6732334,  
              InheritPos = 1,  
              JacketColor = 2,  
              JacketModel = 6734382,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 3,  
              MorphTarget2 = 6,  
              MorphTarget3 = 7,  
              MorphTarget4 = 3,  
              MorphTarget5 = 3,  
              MorphTarget6 = 2,  
              MorphTarget7 = 1,  
              MorphTarget8 = 0,  
              Name = [[Mi]],  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_guard_melee_tank_blunt_f4.creature]],  
              SheetClient = [[basic_zorai_male.creature]],  
              Speed = 0,  
              Tattoo = 1,  
              TrouserColor = 2,  
              TrouserModel = 6733358,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6770478,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2233]],  
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
                InstanceId = [[Client1_2237]],  
                Class = [[Position]],  
                x = 32291.57813,  
                y = -21435.125,  
                z = -21.15625
              }
            },  
            {
              InstanceId = [[Client1_2240]],  
              Class = [[NpcCustom]],  
              Angle = 2.84375,  
              ArmColor = 2,  
              ArmModel = 6733870,  
              Base = [[palette.entities.npcs.guards.z_guard_245]],  
              EyesColor = 3,  
              FeetColor = 2,  
              FeetModel = 6731822,  
              GabaritArmsWidth = 4,  
              GabaritBreastSize = 0,  
              GabaritHeight = 6,  
              GabaritLegsWidth = 8,  
              GabaritTorsoWidth = 6,  
              HairColor = 2,  
              HairType = 6732846,  
              HandsColor = 2,  
              HandsModel = 6732334,  
              InheritPos = 1,  
              JacketColor = 2,  
              JacketModel = 6734382,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 4,  
              MorphTarget2 = 0,  
              MorphTarget3 = 5,  
              MorphTarget4 = 1,  
              MorphTarget5 = 4,  
              MorphTarget6 = 3,  
              MorphTarget7 = 2,  
              MorphTarget8 = 2,  
              Name = [[Ba-Nung]],  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_guard_melee_tank_pierce_f4.creature]],  
              SheetClient = [[basic_zorai_female.creature]],  
              Speed = 0,  
              Tattoo = 4,  
              TrouserColor = 2,  
              TrouserModel = 6733358,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6771246,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2238]],  
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
                InstanceId = [[Client1_2242]],  
                Class = [[Position]],  
                x = 32289.03125,  
                y = -21439.04688,  
                z = -21.125
              }
            },  
            {
              InstanceId = [[Client1_2245]],  
              Class = [[NpcCustom]],  
              Angle = 2.84375,  
              ArmColor = 2,  
              ArmModel = 6733870,  
              Base = [[palette.entities.npcs.guards.z_guard_245]],  
              EyesColor = 3,  
              FeetColor = 2,  
              FeetModel = 6731822,  
              GabaritArmsWidth = 3,  
              GabaritBreastSize = 3,  
              GabaritHeight = 3,  
              GabaritLegsWidth = 7,  
              GabaritTorsoWidth = 2,  
              HairColor = 2,  
              HairType = 6732846,  
              HandsColor = 2,  
              HandsModel = 6732334,  
              InheritPos = 1,  
              JacketColor = 2,  
              JacketModel = 6734382,  
              Level = 0,  
              LinkColor = 1,  
              MorphTarget1 = 6,  
              MorphTarget2 = 7,  
              MorphTarget3 = 0,  
              MorphTarget4 = 6,  
              MorphTarget5 = 0,  
              MorphTarget6 = 0,  
              MorphTarget7 = 1,  
              MorphTarget8 = 6,  
              Name = [[Xei]],  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_guard_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_zorai_female.creature]],  
              Speed = 0,  
              Tattoo = 23,  
              TrouserColor = 2,  
              TrouserModel = 6733358,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6771758,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2243]],  
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
                InstanceId = [[Client1_2247]],  
                Class = [[Position]],  
                x = 32287.48438,  
                y = -21438.46875,  
                z = -21.234375
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_2206]],  
            Class = [[Position]],  
            x = -7.234375,  
            y = -10.5,  
            z = -0.359375
          }
        },  
        {
          InstanceId = [[Client1_2264]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Group 18]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_2262]],  
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
              InstanceId = [[Client1_2250]],  
              Class = [[NpcCustom]],  
              Angle = 2.390625,  
              ArmColor = 3,  
              ArmModel = 6722862,  
              Base = [[palette.entities.npcs.guards.t_guard_245]],  
              EyesColor = 5,  
              FeetColor = 3,  
              FeetModel = 6720814,  
              GabaritArmsWidth = 11,  
              GabaritBreastSize = 8,  
              GabaritHeight = 0,  
              GabaritLegsWidth = 1,  
              GabaritTorsoWidth = 7,  
              HairColor = 3,  
              HairType = 6721838,  
              HandsColor = 3,  
              HandsModel = 6721326,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6723374,  
              Level = 0,  
              LinkColor = 1,  
              MorphTarget1 = 3,  
              MorphTarget2 = 6,  
              MorphTarget3 = 1,  
              MorphTarget4 = 0,  
              MorphTarget5 = 6,  
              MorphTarget6 = 6,  
              MorphTarget7 = 4,  
              MorphTarget8 = 0,  
              Name = [[Ba'Massey]],  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_guard_melee_tank_blunt_f4.creature]],  
              SheetClient = [[basic_tryker_female.creature]],  
              Speed = 0,  
              Tattoo = 26,  
              TrouserColor = 3,  
              TrouserModel = 6722350,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6765614,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2248]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_2335]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_2336]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Wander]],  
                        ActivityZoneId = r2.RefId([[Client1_2179]]),  
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
                InstanceId = [[Client1_2251]],  
                Class = [[Position]],  
                x = 32301.71875,  
                y = -21440.82813,  
                z = -21.4375
              }
            },  
            {
              InstanceId = [[Client1_2259]],  
              Class = [[NpcCustom]],  
              Angle = 2.390625,  
              ArmColor = 3,  
              ArmModel = 6722862,  
              Base = [[palette.entities.npcs.guards.t_guard_245]],  
              EyesColor = 5,  
              FeetColor = 3,  
              FeetModel = 6720814,  
              GabaritArmsWidth = 11,  
              GabaritBreastSize = 8,  
              GabaritHeight = 0,  
              GabaritLegsWidth = 1,  
              GabaritTorsoWidth = 7,  
              HairColor = 3,  
              HairType = 6721838,  
              HandsColor = 3,  
              HandsModel = 6721326,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6723374,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 3,  
              MorphTarget2 = 6,  
              MorphTarget3 = 1,  
              MorphTarget4 = 0,  
              MorphTarget5 = 6,  
              MorphTarget6 = 6,  
              MorphTarget7 = 4,  
              MorphTarget8 = 0,  
              Name = [[Ba'caunin]],  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_guard_melee_tank_blunt_f4.creature]],  
              SheetClient = [[basic_tryker_female.creature]],  
              Speed = 0,  
              Tattoo = 26,  
              TrouserColor = 3,  
              TrouserModel = 6722350,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6765614,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2260]],  
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
                InstanceId = [[Client1_2261]],  
                Class = [[Position]],  
                x = 32301.90625,  
                y = -21442.53125,  
                z = -21.4375
              }
            },  
            {
              InstanceId = [[Client1_2297]],  
              Class = [[NpcCustom]],  
              Angle = 2.390625,  
              ArmColor = 3,  
              ArmModel = 6722862,  
              Base = [[palette.entities.npcs.guards.t_guard_245]],  
              EyesColor = 6,  
              FeetColor = 3,  
              FeetModel = 6720814,  
              GabaritArmsWidth = 5,  
              GabaritBreastSize = 10,  
              GabaritHeight = 14,  
              GabaritLegsWidth = 7,  
              GabaritTorsoWidth = 2,  
              HairColor = 3,  
              HairType = 6721838,  
              HandsColor = 3,  
              HandsModel = 6721326,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6723374,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 0,  
              MorphTarget2 = 4,  
              MorphTarget3 = 3,  
              MorphTarget4 = 0,  
              MorphTarget5 = 7,  
              MorphTarget6 = 7,  
              MorphTarget7 = 4,  
              MorphTarget8 = 3,  
              Name = [[Ba'Massey]],  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_guard_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_tryker_male.creature]],  
              Speed = 0,  
              Tattoo = 21,  
              TrouserColor = 3,  
              TrouserModel = 6722350,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6766638,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2295]],  
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
                InstanceId = [[Client1_2299]],  
                Class = [[Position]],  
                x = 32299.89063,  
                y = -21441.32813,  
                z = -21.328125
              }
            },  
            {
              InstanceId = [[Client1_2302]],  
              Class = [[NpcCustom]],  
              Angle = 2.390625,  
              ArmColor = 3,  
              ArmModel = 6722862,  
              Base = [[palette.entities.npcs.guards.t_guard_245]],  
              EyesColor = 1,  
              FeetColor = 3,  
              FeetModel = 6720814,  
              GabaritArmsWidth = 8,  
              GabaritBreastSize = 14,  
              GabaritHeight = 3,  
              GabaritLegsWidth = 3,  
              GabaritTorsoWidth = 6,  
              HairColor = 3,  
              HairType = 6721838,  
              HandsColor = 3,  
              HandsModel = 6721326,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6723374,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 0,  
              MorphTarget2 = 2,  
              MorphTarget3 = 1,  
              MorphTarget4 = 0,  
              MorphTarget5 = 6,  
              MorphTarget6 = 6,  
              MorphTarget7 = 1,  
              MorphTarget8 = 1,  
              Name = [[O'Darghan]],  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_guard_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_tryker_male.creature]],  
              Speed = 0,  
              Tattoo = 15,  
              TrouserColor = 3,  
              TrouserModel = 6722350,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6766638,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2300]],  
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
                InstanceId = [[Client1_2304]],  
                Class = [[Position]],  
                x = 32303.01563,  
                y = -21439.96875,  
                z = -21.5625
              }
            },  
            {
              InstanceId = [[Client1_2307]],  
              Class = [[NpcCustom]],  
              Angle = 2.390625,  
              ArmColor = 3,  
              ArmModel = 6722862,  
              Base = [[palette.entities.npcs.guards.t_guard_245]],  
              EyesColor = 6,  
              FeetColor = 3,  
              FeetModel = 6720814,  
              GabaritArmsWidth = 12,  
              GabaritBreastSize = 11,  
              GabaritHeight = 2,  
              GabaritLegsWidth = 9,  
              GabaritTorsoWidth = 3,  
              HairColor = 3,  
              HairType = 6721838,  
              HandsColor = 3,  
              HandsModel = 6721326,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6723374,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 1,  
              MorphTarget2 = 1,  
              MorphTarget3 = 6,  
              MorphTarget4 = 7,  
              MorphTarget5 = 3,  
              MorphTarget6 = 7,  
              MorphTarget7 = 4,  
              MorphTarget8 = 2,  
              Name = [[Ba'caunin]],  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_guard_melee_tank_blunt_f4.creature]],  
              SheetClient = [[basic_tryker_female.creature]],  
              Speed = 0,  
              Tattoo = 4,  
              TrouserColor = 3,  
              TrouserModel = 6722350,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6765358,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2305]],  
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
                InstanceId = [[Client1_2309]],  
                Class = [[Position]],  
                x = 32300.79688,  
                y = -21438.92188,  
                z = -21.3125
              }
            },  
            {
              InstanceId = [[Client1_2312]],  
              Class = [[NpcCustom]],  
              Angle = 2.953125,  
              ArmColor = 3,  
              ArmModel = 6712110,  
              Base = [[palette.entities.npcs.guards.m_guard_245]],  
              EyesColor = 6,  
              FeetColor = 3,  
              FeetModel = 6710062,  
              GabaritArmsWidth = 13,  
              GabaritBreastSize = 6,  
              GabaritHeight = 1,  
              GabaritLegsWidth = 2,  
              GabaritTorsoWidth = 1,  
              HairColor = 3,  
              HairType = 6711086,  
              HandsColor = 3,  
              HandsModel = 6710574,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6712622,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 6,  
              MorphTarget2 = 7,  
              MorphTarget3 = 2,  
              MorphTarget4 = 0,  
              MorphTarget5 = 7,  
              MorphTarget6 = 4,  
              MorphTarget7 = 6,  
              MorphTarget8 = 4,  
              Name = [[Sirgio]],  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_guard_melee_tank_pierce_f4.creature]],  
              SheetClient = [[basic_matis_female.creature]],  
              Speed = 0,  
              Tattoo = 26,  
              TrouserColor = 3,  
              TrouserModel = 6711598,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6761006,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2310]],  
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
                InstanceId = [[Client1_2314]],  
                Class = [[Position]],  
                x = 32302.17188,  
                y = -21437.875,  
                z = -21.390625
              }
            },  
            {
              InstanceId = [[Client1_2317]],  
              Class = [[NpcCustom]],  
              Angle = 2.953125,  
              ArmColor = 3,  
              ArmModel = 6712110,  
              Base = [[palette.entities.npcs.guards.m_guard_245]],  
              EyesColor = 5,  
              FeetColor = 3,  
              FeetModel = 6710062,  
              GabaritArmsWidth = 10,  
              GabaritBreastSize = 12,  
              GabaritHeight = 10,  
              GabaritLegsWidth = 12,  
              GabaritTorsoWidth = 1,  
              HairColor = 3,  
              HairType = 6711086,  
              HandsColor = 3,  
              HandsModel = 6710574,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6712622,  
              Level = 0,  
              LinkColor = 1,  
              MorphTarget1 = 7,  
              MorphTarget2 = 1,  
              MorphTarget3 = 5,  
              MorphTarget4 = 3,  
              MorphTarget5 = 5,  
              MorphTarget6 = 5,  
              MorphTarget7 = 4,  
              MorphTarget8 = 0,  
              Name = [[Anigio]],  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_guard_melee_tank_pierce_f4.creature]],  
              SheetClient = [[basic_matis_male.creature]],  
              Speed = 0,  
              Tattoo = 3,  
              TrouserColor = 3,  
              TrouserModel = 6711598,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6761006,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2315]],  
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
                InstanceId = [[Client1_2319]],  
                Class = [[Position]],  
                x = 32299.85938,  
                y = -21437.60938,  
                z = -21.21875
              }
            },  
            {
              InstanceId = [[Client1_2322]],  
              Class = [[NpcCustom]],  
              Angle = 2.953125,  
              ArmColor = 3,  
              ArmModel = 6712110,  
              Base = [[palette.entities.npcs.guards.m_guard_245]],  
              EyesColor = 6,  
              FeetColor = 3,  
              FeetModel = 6710062,  
              GabaritArmsWidth = 12,  
              GabaritBreastSize = 1,  
              GabaritHeight = 4,  
              GabaritLegsWidth = 6,  
              GabaritTorsoWidth = 5,  
              HairColor = 3,  
              HairType = 6711086,  
              HandsColor = 3,  
              HandsModel = 6710574,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6712622,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 1,  
              MorphTarget2 = 2,  
              MorphTarget3 = 5,  
              MorphTarget4 = 7,  
              MorphTarget5 = 7,  
              MorphTarget6 = 5,  
              MorphTarget7 = 3,  
              MorphTarget8 = 4,  
              Name = [[Aniccio]],  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_guard_melee_tank_blunt_f4.creature]],  
              SheetClient = [[basic_matis_male.creature]],  
              Speed = 0,  
              Tattoo = 30,  
              TrouserColor = 3,  
              TrouserModel = 6711598,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6760494,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2320]],  
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
                InstanceId = [[Client1_2324]],  
                Class = [[Position]],  
                x = 32304.78125,  
                y = -21438.54688,  
                z = -21.609375
              }
            },  
            {
              InstanceId = [[Client1_2327]],  
              Class = [[NpcCustom]],  
              Angle = 2.953125,  
              ArmColor = 3,  
              ArmModel = 6712110,  
              Base = [[palette.entities.npcs.guards.m_guard_245]],  
              EyesColor = 1,  
              FeetColor = 3,  
              FeetModel = 6710062,  
              GabaritArmsWidth = 2,  
              GabaritBreastSize = 7,  
              GabaritHeight = 9,  
              GabaritLegsWidth = 1,  
              GabaritTorsoWidth = 4,  
              HairColor = 3,  
              HairType = 6711086,  
              HandsColor = 3,  
              HandsModel = 6710574,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6712622,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 4,  
              MorphTarget2 = 3,  
              MorphTarget3 = 0,  
              MorphTarget4 = 4,  
              MorphTarget5 = 5,  
              MorphTarget6 = 4,  
              MorphTarget7 = 1,  
              MorphTarget8 = 0,  
              Name = [[Gicho]],  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_guard_melee_tank_pierce_f4.creature]],  
              SheetClient = [[basic_matis_male.creature]],  
              Speed = 0,  
              Tattoo = 29,  
              TrouserColor = 3,  
              TrouserModel = 6711598,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6760750,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2325]],  
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
                InstanceId = [[Client1_2329]],  
                Class = [[Position]],  
                x = 32304.64063,  
                y = -21440.59375,  
                z = -21.640625
              }
            },  
            {
              InstanceId = [[Client1_2332]],  
              Class = [[NpcCustom]],  
              Angle = 2.953125,  
              ArmColor = 3,  
              ArmModel = 6712110,  
              Base = [[palette.entities.npcs.guards.m_guard_245]],  
              EyesColor = 6,  
              FeetColor = 3,  
              FeetModel = 6710062,  
              GabaritArmsWidth = 12,  
              GabaritBreastSize = 12,  
              GabaritHeight = 14,  
              GabaritLegsWidth = 1,  
              GabaritTorsoWidth = 7,  
              HairColor = 3,  
              HairType = 6711086,  
              HandsColor = 3,  
              HandsModel = 6710574,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6712622,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 2,  
              MorphTarget2 = 4,  
              MorphTarget3 = 1,  
              MorphTarget4 = 2,  
              MorphTarget5 = 0,  
              MorphTarget6 = 4,  
              MorphTarget7 = 3,  
              MorphTarget8 = 6,  
              Name = [[Miarni]],  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_guard_melee_tank_pierce_f4.creature]],  
              SheetClient = [[basic_matis_male.creature]],  
              Speed = 0,  
              Tattoo = 28,  
              TrouserColor = 3,  
              TrouserModel = 6711598,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6760750,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2330]],  
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
                InstanceId = [[Client1_2334]],  
                Class = [[Position]],  
                x = 32303.64063,  
                y = -21442.26563,  
                z = -21.546875
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_2263]],  
            Class = [[Position]],  
            x = -6.953125,  
            y = -18.390625,  
            z = 0.453125
          }
        },  
        {
          InstanceId = [[Client1_2488]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Group 19]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_2486]],  
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
              InstanceId = [[Client1_2420]],  
              Class = [[NpcCreature]],  
              Angle = -1.36185801,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ckbif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kinrey]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2421]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_2550]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_2657]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_2528]]),  
                        Action = {
                          InstanceId = [[Client1_2656]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_2551]],  
                      Class = [[EventType]],  
                      Type = [[group death]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_2654]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_2655]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Guard Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_2179]]),  
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
                InstanceId = [[Client1_2422]],  
                Class = [[Position]],  
                x = 32265.46875,  
                y = -21348.46875,  
                z = 0.140625
              }
            },  
            {
              InstanceId = [[Client1_2341]],  
              Class = [[NpcCreature]],  
              Angle = -1.126889825,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ckgpf7]],  
              InheritPos = 1,  
              Name = [[Kibakoo]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2339]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_2545]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_2548]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_2528]]),  
                        Action = {
                          InstanceId = [[Client1_2547]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_2546]],  
                      Class = [[EventType]],  
                      Type = [[group death]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_2523]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_2524]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Guard Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_2179]]),  
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
                InstanceId = [[Client1_2342]],  
                Class = [[Position]],  
                x = 32252.84375,  
                y = -21323.3125,  
                z = 4.25
              }
            },  
            {
              InstanceId = [[Client1_2387]],  
              Class = [[NpcCreature]],  
              Angle = -1.309345961,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ckaif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kidinak]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2385]],  
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
                InstanceId = [[Client1_2388]],  
                Class = [[Position]],  
                x = 32260.98438,  
                y = -21341.21875,  
                z = 3.109375
              }
            },  
            {
              InstanceId = [[Client1_2396]],  
              Class = [[NpcCreature]],  
              Angle = -1.309345961,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ckaif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kidinak]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2397]],  
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
                InstanceId = [[Client1_2398]],  
                Class = [[Position]],  
                x = 32263.89063,  
                y = -21343.10938,  
                z = 2.34375
              }
            },  
            {
              InstanceId = [[Client1_2406]],  
              Class = [[NpcCreature]],  
              Angle = -1.309345961,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ckaif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kidinak]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2407]],  
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
                InstanceId = [[Client1_2408]],  
                Class = [[Position]],  
                x = 32266.3125,  
                y = -21339.21875,  
                z = 3.03125
              }
            },  
            {
              InstanceId = [[Client1_2362]],  
              Class = [[NpcCreature]],  
              Angle = -1.037345767,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ckdif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kincher]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2363]],  
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
                InstanceId = [[Client1_2364]],  
                Class = [[Position]],  
                x = 32271.5,  
                y = -21335.64063,  
                z = 3
              }
            },  
            {
              InstanceId = [[Client1_2353]],  
              Class = [[NpcCreature]],  
              Angle = -1.037345767,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ckdif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kincher]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2351]],  
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
                InstanceId = [[Client1_2354]],  
                Class = [[Position]],  
                x = 32270.89063,  
                y = -21325.60938,  
                z = 3.9375
              }
            },  
            {
              InstanceId = [[Client1_2382]],  
              Class = [[NpcCreature]],  
              Angle = -1.037345767,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ckdif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kincher]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2383]],  
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
                InstanceId = [[Client1_2384]],  
                Class = [[Position]],  
                x = 32254.10938,  
                y = -21343.625,  
                z = 2.828125
              }
            },  
            {
              InstanceId = [[Client1_2372]],  
              Class = [[NpcCreature]],  
              Angle = -1.037345767,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ckdif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kincher]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2373]],  
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
                InstanceId = [[Client1_2374]],  
                Class = [[Position]],  
                x = 32243.20313,  
                y = -21332.03125,  
                z = 0.65625
              }
            },  
            {
              InstanceId = [[Client1_2483]],  
              Class = [[NpcCreature]],  
              Angle = -1.24817121,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ckfrf3]],  
              InheritPos = 1,  
              Name = [[Great Commander Kirosta]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2484]],  
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
                InstanceId = [[Client1_2485]],  
                Class = [[Position]],  
                x = 32238.85938,  
                y = -21316.28125,  
                z = 4.96875
              }
            },  
            {
              InstanceId = [[Client1_2473]],  
              Class = [[NpcCreature]],  
              Angle = -1.24817121,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ckfrf3]],  
              InheritPos = 1,  
              Name = [[Great Commander Kirosta]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2474]],  
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
                InstanceId = [[Client1_2475]],  
                Class = [[Position]],  
                x = 32236.21875,  
                y = -21330.67188,  
                z = 2.734375
              }
            },  
            {
              InstanceId = [[Client1_2435]],  
              Class = [[NpcCreature]],  
              Angle = -1.36185801,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ckeif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kipucker]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2433]],  
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
                InstanceId = [[Client1_2436]],  
                Class = [[Position]],  
                x = 32239.73438,  
                y = -21340.39063,  
                z = 0.34375
              }
            },  
            {
              InstanceId = [[Client1_2430]],  
              Class = [[NpcCreature]],  
              Angle = -1.36185801,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ckbif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kinrey]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2431]],  
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
                InstanceId = [[Client1_2432]],  
                Class = [[Position]],  
                x = 32256.25,  
                y = -21350.17188,  
                z = -0.265625
              }
            },  
            {
              InstanceId = [[Client1_2411]],  
              Class = [[NpcCreature]],  
              Angle = -1.36185801,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ckbif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kinrey]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2409]],  
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
                InstanceId = [[Client1_2412]],  
                Class = [[Position]],  
                x = 32274.625,  
                y = -21345.98438,  
                z = -0.015625
              }
            },  
            {
              InstanceId = [[Client1_2444]],  
              Class = [[NpcCreature]],  
              Angle = -1.36185801,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ckeif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kipucker]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2445]],  
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
                InstanceId = [[Client1_2446]],  
                Class = [[Position]],  
                x = 32282.57813,  
                y = -21331.75,  
                z = 1.296875
              }
            },  
            {
              InstanceId = [[Client1_2449]],  
              Class = [[NpcCreature]],  
              Angle = -1.24817121,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ckfrf3]],  
              InheritPos = 1,  
              Name = [[Great Commander Kirosta]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2447]],  
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
                InstanceId = [[Client1_2450]],  
                Class = [[Position]],  
                x = 32280.84375,  
                y = -21322.57813,  
                z = 1.328125
              }
            },  
            {
              InstanceId = [[Client1_2463]],  
              Class = [[NpcCreature]],  
              Angle = -1.24817121,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ckfrf3]],  
              InheritPos = 1,  
              Name = [[Great Commander Kirosta]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2464]],  
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
                InstanceId = [[Client1_2465]],  
                Class = [[Position]],  
                x = 32277.125,  
                y = -21313.01563,  
                z = 4.421875
              }
            },  
            {
              InstanceId = [[Client1_2345]],  
              Class = [[NpcCreature]],  
              Angle = -1.342889905,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ckipf7]],  
              InheritPos = 1,  
              Name = [[Kizokoo]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2343]],  
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
                InstanceId = [[Client1_2346]],  
                Class = [[Position]],  
                x = 32264.46875,  
                y = -21317.53125,  
                z = 4.09375
              }
            },  
            {
              InstanceId = [[Client1_2496]],  
              Class = [[NpcCreature]],  
              Angle = -1.331501126,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ckfpf7]],  
              InheritPos = 1,  
              Name = [[Kirokoo]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2494]],  
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
                InstanceId = [[Client1_2497]],  
                Class = [[Position]],  
                x = 32253.84375,  
                y = -21335,  
                z = 1.15625
              }
            },  
            {
              InstanceId = [[Client1_2500]],  
              Class = [[NpcCreature]],  
              Angle = -1.379501104,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ckbpf7]],  
              InheritPos = 1,  
              Name = [[Kinkoo]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2498]],  
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
                InstanceId = [[Client1_2501]],  
                Class = [[Position]],  
                x = 32263.71875,  
                y = -21333.28125,  
                z = 1.921875
              }
            },  
            {
              InstanceId = [[Client1_2588]],  
              Class = [[NpcCreature]],  
              Angle = -1.309345961,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ckaif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kidinak]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2589]],  
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
                InstanceId = [[Client1_2590]],  
                Class = [[Position]],  
                x = 32270,  
                y = -21345.76563,  
                z = 1.78125
              }
            },  
            {
              InstanceId = [[Client1_2568]],  
              Class = [[NpcCreature]],  
              Angle = -1.309345961,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ckaif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kidinak]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2569]],  
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
                InstanceId = [[Client1_2570]],  
                Class = [[Position]],  
                x = 32267.78125,  
                y = -21342.67188,  
                z = 1.84375
              }
            },  
            {
              InstanceId = [[Client1_2578]],  
              Class = [[NpcCreature]],  
              Angle = -1.309345961,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ckaif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kidinak]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2579]],  
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
                InstanceId = [[Client1_2580]],  
                Class = [[Position]],  
                x = 32260.42188,  
                y = -21344.29688,  
                z = 1.5
              }
            },  
            {
              InstanceId = [[Client1_2601]],  
              Class = [[NpcCreature]],  
              Angle = -1.309345961,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ckaif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kidinak]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2602]],  
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
                InstanceId = [[Client1_2603]],  
                Class = [[Position]],  
                x = 32260.40625,  
                y = -21347.57813,  
                z = 1.5625
              }
            },  
            {
              InstanceId = [[Client1_2618]],  
              Class = [[NpcCreature]],  
              Angle = -1.36185801,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ckbif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kinrey]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2619]],  
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
                InstanceId = [[Client1_2620]],  
                Class = [[Position]],  
                x = 32282.98438,  
                y = -21344.15625,  
                z = 1.71875
              }
            },  
            {
              InstanceId = [[Client1_2628]],  
              Class = [[NpcCreature]],  
              Angle = -1.36185801,  
              AutoSpawn = 0,  
              Base = [[palette.entities.creatures.ckbif3]],  
              InheritPos = 1,  
              Name = [[Great Overlord Kinrey]],  
              NoRespawn = 1,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2629]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_4801]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_4802]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Guard Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_2179]]),  
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
                InstanceId = [[Client1_2630]],  
                Class = [[Position]],  
                x = 32245.96875,  
                y = -21351.65625,  
                z = -0.40625
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_2487]],  
            Class = [[Position]],  
            x = -1.75,  
            y = 18.078125,  
            z = 2
          }
        },  
        {
          InstanceId = [[Client1_2506]],  
          Class = [[ChatSequence]],  
          Active = 0,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Be-ci]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_2504]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_4804]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_4807]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2506]]),  
                    Action = {
                      InstanceId = [[Client1_4806]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_4805]],  
                  Class = [[EventType]],  
                  Type = [[end of dialog]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_2507]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_2508]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_2525]],  
                  Who = r2.RefId([[Client1_2147]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_2509]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_2510]],  
                  Class = [[ChatAction]],  
                  Emote = [[Alert]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_2514]],  
                  Who = r2.RefId([[Client1_1031]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_2512]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_2513]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_2515]],  
                  Who = r2.RefId([[Client1_2147]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_2646]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_2647]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_2648]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_2505]],  
            Class = [[Position]],  
            x = 32266.17188,  
            y = -21480.95313,  
            z = -21
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_2528]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Be-ci 2]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_2526]],  
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
                    Entity = r2.RefId([[Client1_2552]]),  
                    Action = {
                      InstanceId = [[Client1_2652]],  
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
              InstanceId = [[Client1_2529]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_2530]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_2531]],  
                  Who = r2.RefId([[Client1_2147]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_2532]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_2533]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_2536]],  
                  Who = r2.RefId([[Client1_2147]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_2534]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_2535]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_2539]],  
                  Who = r2.RefId([[Client1_2147]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_2537]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_2538]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_2542]],  
                  Who = r2.RefId([[Client1_2147]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_2540]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_2541]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_2543]],  
                  Who = r2.RefId([[Client1_2147]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_2558]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_2559]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_2560]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_2527]],  
            Class = [[Position]],  
            x = 32262.54688,  
            y = -21470.03125,  
            z = -21
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_2552]],  
          Class = [[Timer]],  
          Active = 0,  
          Base = [[palette.entities.botobjects.timer]],  
          Cyclic = 0,  
          InheritPos = 1,  
          Minutes = 0,  
          Name = [[Act 2 end]],  
          Secondes = 10,  
          Behavior = {
            InstanceId = [[Client1_2553]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_2637]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_2640]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2633]]),  
                    Action = {
                      InstanceId = [[Client1_2639]],  
                      Class = [[ActionType]],  
                      Type = [[Start Act]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_2638]],  
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
            InstanceId = [[Client1_2554]],  
            Class = [[Position]],  
            x = 32247.48438,  
            y = -21506.57813,  
            z = -23
          }
        },  
        {
          InstanceId = [[Client1_5760]],  
          Class = [[ZoneTrigger]],  
          Active = 0,  
          Base = [[palette.entities.botobjects.trigger_zone]],  
          Cyclic = 0,  
          InheritPos = 1,  
          Name = [[Restart mission]],  
          _Zone = [[Client1_5764]],  
          Behavior = {
            InstanceId = [[Client1_5761]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_5778]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_5781]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_891]]),  
                    Action = {
                      InstanceId = [[Client1_5780]],  
                      Class = [[ActionType]],  
                      Type = [[Start Act]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_5779]],  
                  Class = [[EventType]],  
                  Type = [[On Player Arrived]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_5764]],  
              Class = [[Region]],  
              Deletable = 0,  
              InheritPos = 1,  
              Name = [[Places 1]],  
              Points = {
                {
                  InstanceId = [[Client1_5766]],  
                  Class = [[RegionVertex]],  
                  Deletable = 0,  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_5767]],  
                    Class = [[Position]],  
                    x = 34.140625,  
                    y = 2.421875,  
                    z = 0.84375
                  }
                },  
                {
                  InstanceId = [[Client1_5769]],  
                  Class = [[RegionVertex]],  
                  Deletable = 0,  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_5770]],  
                    Class = [[Position]],  
                    x = 8.8125,  
                    y = 46.078125,  
                    z = -2.296875
                  }
                },  
                {
                  InstanceId = [[Client1_5772]],  
                  Class = [[RegionVertex]],  
                  Deletable = 0,  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_5773]],  
                    Class = [[Position]],  
                    x = -29.40625,  
                    y = 7.046875,  
                    z = -3.46875
                  }
                },  
                {
                  InstanceId = [[Client1_5775]],  
                  Class = [[RegionVertex]],  
                  Deletable = 0,  
                  InheritPos = 1,  
                  Position = {
                    InstanceId = [[Client1_5776]],  
                    Class = [[Position]],  
                    x = 10.6875,  
                    y = -12.78125,  
                    z = -0.46875
                  }
                }
              },  
              Position = {
                InstanceId = [[Client1_5763]],  
                Class = [[Position]],  
                x = 8.734375,  
                y = -2.6875,  
                z = -0.4375
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_5762]],  
            Class = [[Position]],  
            x = 32686.9375,  
            y = -21088.95313,  
            z = 7.59375
          }
        }
      },  
      Position = {
        InstanceId = [[Client1_890]],  
        Class = [[Position]],  
        x = 0,  
        y = 0,  
        z = 0
      }
    },  
    {
      InstanceId = [[Client1_2633]],  
      Class = [[Act]],  
      Version = 6,  
      InheritPos = 1,  
      LocationId = [[Client1_2635]],  
      ManualWeather = 1,  
      Name = [[Act 3]],  
      Season = 0,  
      ShortDescription = [[A group of bandits stand between the group and safety from the Kitin, they must be dealt with!]],  
      Title = [[]],  
      WeatherValue = 0,  
      ActivitiesIds = {
      },  
      Behavior = {
        InstanceId = [[Client1_2631]],  
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
          InstanceId = [[Client1_2634]],  
          Class = [[DefaultFeature]],  
          Components = {
            {
              InstanceId = [[Client1_2664]],  
              Class = [[NpcCustom]],  
              Angle = -0.0625,  
              ArmColor = 4,  
              ArmModel = 6722862,  
              Base = [[palette.entities.npcs.guards.t_guard_245]],  
              EyesColor = 3,  
              FeetColor = 4,  
              FeetModel = 6720814,  
              GabaritArmsWidth = 6,  
              GabaritBreastSize = 6,  
              GabaritHeight = 6,  
              GabaritLegsWidth = 8,  
              GabaritTorsoWidth = 4,  
              HairColor = 4,  
              HairType = 6721838,  
              HandsColor = 4,  
              HandsModel = 6721326,  
              InheritPos = 1,  
              JacketColor = 4,  
              JacketModel = 6723374,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 3,  
              MorphTarget2 = 2,  
              MorphTarget3 = 3,  
              MorphTarget4 = 2,  
              MorphTarget5 = 4,  
              MorphTarget6 = 3,  
              MorphTarget7 = 0,  
              MorphTarget8 = 3,  
              Name = [[Gale Brapie]],  
              NoRespawn = 1,  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_guard_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_tryker_female.creature]],  
              Speed = 1,  
              Tattoo = 16,  
              TrouserColor = 4,  
              TrouserModel = 6722350,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6766894,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2662]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_3858]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_3861]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_3848]]),  
                        Action = {
                          InstanceId = [[Client1_3860]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_3863]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_3851]]),  
                        Action = {
                          InstanceId = [[Client1_3862]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_3865]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_3854]]),  
                        Action = {
                          InstanceId = [[Client1_3864]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_4786]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_4775]]),  
                        Action = {
                          InstanceId = [[Client1_4785]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_3859]],  
                      Class = [[EventType]],  
                      Type = [[end of activity sequence]],  
                      Value = r2.RefId([[Client1_3780]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4183]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_4186]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_3848]]),  
                        Action = {
                          InstanceId = [[Client1_4185]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_4188]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_3851]]),  
                        Action = {
                          InstanceId = [[Client1_4187]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_4190]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_3854]]),  
                        Action = {
                          InstanceId = [[Client1_4189]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_4784]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_4775]]),  
                        Action = {
                          InstanceId = [[Client1_4783]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_4184]],  
                      Class = [[EventType]],  
                      Type = [[end of activity sequence]],  
                      Value = r2.RefId([[Client1_3552]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_3077]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                    }
                  },  
                  {
                    InstanceId = [[Client1_3078]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3079]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3195]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3236]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3237]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3055]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3552]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3553]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3385]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3780]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3781]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3655]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3872]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3873]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3044]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_4167]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_4168]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3925]]),  
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
                InstanceId = [[Client1_2665]],  
                Class = [[Position]],  
                x = 36251.79688,  
                y = -1088.71875,  
                z = -21.3125
              }
            },  
            {
              InstanceId = [[Client1_2668]],  
              Class = [[NpcCustom]],  
              Angle = -1.4375,  
              ArmColor = 0,  
              ArmModel = 6733614,  
              Base = [[palette.entities.npcs.guards.z_guard_245]],  
              EyesColor = 2,  
              FeetColor = 0,  
              FeetModel = 6731566,  
              GabaritArmsWidth = 9,  
              GabaritBreastSize = 12,  
              GabaritHeight = 14,  
              GabaritLegsWidth = 6,  
              GabaritTorsoWidth = 7,  
              HairColor = 0,  
              HairType = 6732590,  
              HandsColor = 0,  
              HandsModel = 6732078,  
              InheritPos = 1,  
              JacketColor = 0,  
              JacketModel = 6734126,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 3,  
              MorphTarget2 = 1,  
              MorphTarget3 = 0,  
              MorphTarget4 = 1,  
              MorphTarget5 = 6,  
              MorphTarget6 = 5,  
              MorphTarget7 = 5,  
              MorphTarget8 = 1,  
              Name = [[Be-ci Kuani]],  
              NoRespawn = 1,  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_guard_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_zorai_male.creature]],  
              Speed = 1,  
              Tattoo = 3,  
              TrouserColor = 0,  
              TrouserModel = 6733102,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6772014,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2666]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_3226]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_3229]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_3222]]),  
                        Action = {
                          InstanceId = [[Client1_3228]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_3273]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_3266]]),  
                        Action = {
                          InstanceId = [[Client1_3272]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_3275]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_3269]]),  
                        Action = {
                          InstanceId = [[Client1_3274]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_4780]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_4775]]),  
                        Action = {
                          InstanceId = [[Client1_4779]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_3227]],  
                      Class = [[EventType]],  
                      Type = [[end of activity sequence]],  
                      Value = r2.RefId([[Client1_3069]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4174]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_4177]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_3266]]),  
                        Action = {
                          InstanceId = [[Client1_4176]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_4179]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_3269]]),  
                        Action = {
                          InstanceId = [[Client1_4178]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_4181]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_3222]]),  
                        Action = {
                          InstanceId = [[Client1_4180]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_4782]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_4775]]),  
                        Action = {
                          InstanceId = [[Client1_4781]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_4175]],  
                      Class = [[EventType]],  
                      Type = [[end of activity sequence]],  
                      Value = r2.RefId([[Client1_4165]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_3068]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                    }
                  },  
                  {
                    InstanceId = [[Client1_3069]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3070]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3177]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3234]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3235]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3055]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3546]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3547]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3027]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3771]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3772]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_2996]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3870]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3871]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3044]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_4165]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_4166]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3010]]),  
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
                InstanceId = [[Client1_2669]],  
                Class = [[Position]],  
                x = 36258.90625,  
                y = -1095.703125,  
                z = -16.40625
              }
            },  
            {
              InstanceId = [[Client1_2660]],  
              Class = [[NpcCustom]],  
              Angle = -0.03125,  
              ArmColor = 4,  
              ArmModel = 6701614,  
              Base = [[palette.entities.npcs.guards.f_guard_245]],  
              EyesColor = 0,  
              FeetColor = 4,  
              FeetModel = 6699566,  
              GabaritArmsWidth = 7,  
              GabaritBreastSize = 5,  
              GabaritHeight = 1,  
              GabaritLegsWidth = 8,  
              GabaritTorsoWidth = 2,  
              HairColor = 4,  
              HairType = 6700590,  
              HandsColor = 4,  
              HandsModel = 6700078,  
              InheritPos = 1,  
              JacketColor = 4,  
              JacketModel = 6702126,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 4,  
              MorphTarget2 = 3,  
              MorphTarget3 = 6,  
              MorphTarget4 = 6,  
              MorphTarget5 = 1,  
              MorphTarget6 = 6,  
              MorphTarget7 = 3,  
              MorphTarget8 = 2,  
              Name = [[Abyrixius Iodix]],  
              NoRespawn = 1,  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_guard_melee_tank_blunt_f4.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 1,  
              Tattoo = 28,  
              TrouserColor = 4,  
              TrouserModel = 6701102,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6755374,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2658]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_3071]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                    }
                  },  
                  {
                    InstanceId = [[Client1_3072]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3073]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3116]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3230]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3231]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3055]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3550]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3551]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3349]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3773]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3776]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3631]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3866]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3867]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3044]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_4171]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_4172]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3997]]),  
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
                InstanceId = [[Client1_2661]],  
                Class = [[Position]],  
                x = 36251.01563,  
                y = -1092.625,  
                z = -20.984375
              }
            },  
            {
              InstanceId = [[Client1_2771]],  
              Class = [[NpcCustom]],  
              Angle = -0.25,  
              ArmColor = 4,  
              ArmModel = 6712110,  
              Base = [[palette.entities.npcs.guards.m_guard_245]],  
              EyesColor = 3,  
              FeetColor = 4,  
              FeetModel = 6710062,  
              GabaritArmsWidth = 7,  
              GabaritBreastSize = 4,  
              GabaritHeight = 14,  
              GabaritLegsWidth = 8,  
              GabaritTorsoWidth = 5,  
              HairColor = 4,  
              HairType = 6711086,  
              HandsColor = 4,  
              HandsModel = 6710574,  
              InheritPos = 1,  
              JacketColor = 4,  
              JacketModel = 6712622,  
              Level = 3,  
              LinkColor = 1,  
              MorphTarget1 = 2,  
              MorphTarget2 = 2,  
              MorphTarget3 = 2,  
              MorphTarget4 = 4,  
              MorphTarget5 = 3,  
              MorphTarget6 = 3,  
              MorphTarget7 = 4,  
              MorphTarget8 = 3,  
              Name = [[Anisti Pillo]],  
              NoRespawn = 1,  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_guard_melee_tank_pierce_f4.creature]],  
              SheetClient = [[basic_matis_male.creature]],  
              Speed = 1,  
              Tattoo = 14,  
              TrouserColor = 4,  
              TrouserModel = 6711598,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6760750,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2769]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_4384]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_4387]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_4312]]),  
                        Action = {
                          InstanceId = [[Client1_4386]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_4389]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_4315]]),  
                        Action = {
                          InstanceId = [[Client1_4388]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_4391]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_4318]]),  
                        Action = {
                          InstanceId = [[Client1_4390]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_4788]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_4775]]),  
                        Action = {
                          InstanceId = [[Client1_4787]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_4385]],  
                      Class = [[EventType]],  
                      Type = [[end of activity sequence]],  
                      Value = r2.RefId([[Client1_4309]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4425]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_4428]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_4312]]),  
                        Action = {
                          InstanceId = [[Client1_4427]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_4430]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_4315]]),  
                        Action = {
                          InstanceId = [[Client1_4429]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_4432]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_4318]]),  
                        Action = {
                          InstanceId = [[Client1_4431]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_4790]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_4775]]),  
                        Action = {
                          InstanceId = [[Client1_4789]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_4426]],  
                      Class = [[EventType]],  
                      Type = [[end of activity sequence]],  
                      Value = r2.RefId([[Client1_4422]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_3074]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                    }
                  },  
                  {
                    InstanceId = [[Client1_3075]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3076]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3213]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3232]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3233]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3055]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3548]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3549]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3313]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3778]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3779]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3607]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3868]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3869]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3044]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_4169]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_4170]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3961]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_4309]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_4311]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_4287]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_4422]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_4423]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_4408]]),  
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
                InstanceId = [[Client1_2773]],  
                Class = [[Position]],  
                x = 36251.0625,  
                y = -1090.515625,  
                z = -21.375
              }
            }
          }
        },  
        {
          InstanceId = [[Client1_2683]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Fyros Group]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_2681]],  
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
              InstanceId = [[Client1_2675]],  
              Class = [[NpcCustom]],  
              Angle = -0.0625,  
              ArmColor = 3,  
              ArmModel = 6701614,  
              Base = [[palette.entities.npcs.guards.f_guard_245]],  
              EyesColor = 7,  
              FeetColor = 3,  
              FeetModel = 6699566,  
              GabaritArmsWidth = 0,  
              GabaritBreastSize = 3,  
              GabaritHeight = 4,  
              GabaritLegsWidth = 1,  
              GabaritTorsoWidth = 3,  
              HairColor = 3,  
              HairType = 6700590,  
              HandsColor = 3,  
              HandsModel = 6700078,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6702126,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 2,  
              MorphTarget2 = 1,  
              MorphTarget3 = 1,  
              MorphTarget4 = 1,  
              MorphTarget5 = 3,  
              MorphTarget6 = 1,  
              MorphTarget7 = 4,  
              MorphTarget8 = 6,  
              Name = [[Zenix]],  
              NoRespawn = 1,  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_guard_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 1,  
              Tattoo = 0,  
              TrouserColor = 3,  
              TrouserModel = 6701102,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6756398,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2673]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_4375]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_4378]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_4312]]),  
                        Action = {
                          InstanceId = [[Client1_4377]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_4380]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_4318]]),  
                        Action = {
                          InstanceId = [[Client1_4379]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_4382]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_4315]]),  
                        Action = {
                          InstanceId = [[Client1_4381]],  
                          Class = [[ActionType]],  
                          Type = [[activate]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_4792]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_4775]]),  
                        Action = {
                          InstanceId = [[Client1_4791]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_4376]],  
                      Class = [[EventType]],  
                      Type = [[end of activity sequence]],  
                      Value = r2.RefId([[Client1_4307]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_3083]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                    }
                  },  
                  {
                    InstanceId = [[Client1_3084]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3085]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_2988]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3238]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3239]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3055]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3240]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3556]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3493]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3784]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3795]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3727]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3874]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3875]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3044]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3876]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_4158]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_4033]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_4307]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_4308]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_4248]]),  
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
                InstanceId = [[Client1_2676]],  
                Class = [[Position]],  
                x = 36249.64063,  
                y = -1091.078125,  
                z = -21.734375
              }
            },  
            {
              InstanceId = [[Client1_2679]],  
              Class = [[NpcCustom]],  
              Angle = -0.0625,  
              ArmColor = 3,  
              ArmModel = 6701614,  
              Base = [[palette.entities.npcs.guards.f_guard_245]],  
              EyesColor = 2,  
              FeetColor = 3,  
              FeetModel = 6699566,  
              GabaritArmsWidth = 3,  
              GabaritBreastSize = 2,  
              GabaritHeight = 5,  
              GabaritLegsWidth = 3,  
              GabaritTorsoWidth = 2,  
              HairColor = 3,  
              HairType = 6700590,  
              HandsColor = 3,  
              HandsModel = 6700078,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6702126,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 4,  
              MorphTarget2 = 3,  
              MorphTarget3 = 5,  
              MorphTarget4 = 6,  
              MorphTarget5 = 6,  
              MorphTarget6 = 2,  
              MorphTarget7 = 5,  
              MorphTarget8 = 6,  
              Name = [[Ganix]],  
              NoRespawn = 1,  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_guard_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 1,  
              Tattoo = 15,  
              TrouserColor = 3,  
              TrouserModel = 6701102,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6756142,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2677]],  
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
                InstanceId = [[Client1_2680]],  
                Class = [[Position]],  
                x = 36243.71875,  
                y = -1089.3125,  
                z = -23.109375
              }
            },  
            {
              InstanceId = [[Client1_2686]],  
              Class = [[NpcCustom]],  
              Angle = -0.0625,  
              ArmColor = 3,  
              ArmModel = 6701614,  
              Base = [[palette.entities.npcs.guards.f_guard_245]],  
              EyesColor = 1,  
              FeetColor = 3,  
              FeetModel = 6699566,  
              GabaritArmsWidth = 8,  
              GabaritBreastSize = 0,  
              GabaritHeight = 14,  
              GabaritLegsWidth = 14,  
              GabaritTorsoWidth = 0,  
              HairColor = 3,  
              HairType = 6700590,  
              HandsColor = 3,  
              HandsModel = 6700078,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6702126,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 0,  
              MorphTarget2 = 1,  
              MorphTarget3 = 2,  
              MorphTarget4 = 3,  
              MorphTarget5 = 6,  
              MorphTarget6 = 2,  
              MorphTarget7 = 2,  
              MorphTarget8 = 5,  
              Name = [[Dyla]],  
              NoRespawn = 1,  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_guard_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 1,  
              Tattoo = 23,  
              TrouserColor = 3,  
              TrouserModel = 6701102,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6756910,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2684]],  
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
                InstanceId = [[Client1_2688]],  
                Class = [[Position]],  
                x = 36246.01563,  
                y = -1089.890625,  
                z = -22.703125
              }
            },  
            {
              InstanceId = [[Client1_2691]],  
              Class = [[NpcCustom]],  
              Angle = -0.0625,  
              ArmColor = 3,  
              ArmModel = 6701614,  
              Base = [[palette.entities.npcs.guards.f_guard_245]],  
              EyesColor = 7,  
              FeetColor = 3,  
              FeetModel = 6699566,  
              GabaritArmsWidth = 13,  
              GabaritBreastSize = 11,  
              GabaritHeight = 14,  
              GabaritLegsWidth = 10,  
              GabaritTorsoWidth = 2,  
              HairColor = 3,  
              HairType = 6700590,  
              HandsColor = 3,  
              HandsModel = 6700078,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6702126,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 0,  
              MorphTarget2 = 7,  
              MorphTarget3 = 3,  
              MorphTarget4 = 7,  
              MorphTarget5 = 2,  
              MorphTarget6 = 2,  
              MorphTarget7 = 4,  
              MorphTarget8 = 3,  
              Name = [[Meros]],  
              NoRespawn = 1,  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_guard_melee_tank_pierce_f4.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 1,  
              Tattoo = 2,  
              TrouserColor = 3,  
              TrouserModel = 6701102,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6755630,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2689]],  
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
                InstanceId = [[Client1_2693]],  
                Class = [[Position]],  
                x = 36248.09375,  
                y = -1090.65625,  
                z = -22.125
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_2682]],  
            Class = [[Position]],  
            x = 0,  
            y = 0,  
            z = 0
          }
        },  
        {
          InstanceId = [[Client1_2704]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Matis Group]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_2702]],  
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
              InstanceId = [[Client1_2696]],  
              Class = [[NpcCustom]],  
              Angle = -0.0625,  
              ArmColor = 3,  
              ArmModel = 6712110,  
              Base = [[palette.entities.npcs.guards.m_guard_245]],  
              EyesColor = 5,  
              FeetColor = 3,  
              FeetModel = 6710062,  
              GabaritArmsWidth = 10,  
              GabaritBreastSize = 14,  
              GabaritHeight = 13,  
              GabaritLegsWidth = 11,  
              GabaritTorsoWidth = 6,  
              HairColor = 3,  
              HairType = 6711086,  
              HandsColor = 3,  
              HandsModel = 6710574,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6712622,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 2,  
              MorphTarget2 = 3,  
              MorphTarget3 = 6,  
              MorphTarget4 = 2,  
              MorphTarget5 = 0,  
              MorphTarget6 = 6,  
              MorphTarget7 = 3,  
              MorphTarget8 = 4,  
              Name = [[Stassi]],  
              NoRespawn = 1,  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_guard_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_matis_female.creature]],  
              Speed = 1,  
              Tattoo = 10,  
              TrouserColor = 3,  
              TrouserModel = 6711598,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6761262,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2694]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_3086]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                    }
                  },  
                  {
                    InstanceId = [[Client1_3087]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3088]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3124]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3241]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3242]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3055]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3557]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3558]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3457]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3798]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3799]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3703]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3877]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3878]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3044]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_4161]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_4162]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_4105]]),  
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
                InstanceId = [[Client1_2697]],  
                Class = [[Position]],  
                x = 36249.6875,  
                y = -1088.875,  
                z = -21.921875
              }
            },  
            {
              InstanceId = [[Client1_2700]],  
              Class = [[NpcCustom]],  
              Angle = -0.0625,  
              ArmColor = 3,  
              ArmModel = 6712110,  
              Base = [[palette.entities.npcs.guards.m_guard_245]],  
              EyesColor = 0,  
              FeetColor = 3,  
              FeetModel = 6710062,  
              GabaritArmsWidth = 13,  
              GabaritBreastSize = 7,  
              GabaritHeight = 7,  
              GabaritLegsWidth = 10,  
              GabaritTorsoWidth = 1,  
              HairColor = 3,  
              HairType = 6711086,  
              HandsColor = 3,  
              HandsModel = 6710574,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6712622,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 5,  
              MorphTarget2 = 2,  
              MorphTarget3 = 0,  
              MorphTarget4 = 4,  
              MorphTarget5 = 4,  
              MorphTarget6 = 2,  
              MorphTarget7 = 7,  
              MorphTarget8 = 5,  
              Name = [[Lirgio]],  
              NoRespawn = 1,  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_guard_melee_tank_pierce_f4.creature]],  
              SheetClient = [[basic_matis_male.creature]],  
              Speed = 1,  
              Tattoo = 11,  
              TrouserColor = 3,  
              TrouserModel = 6711598,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6761006,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2698]],  
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
                InstanceId = [[Client1_2701]],  
                Class = [[Position]],  
                x = 36248.03125,  
                y = -1088.328125,  
                z = -22.390625
              }
            },  
            {
              InstanceId = [[Client1_2707]],  
              Class = [[NpcCustom]],  
              Angle = -0.0625,  
              ArmColor = 3,  
              ArmModel = 6712110,  
              Base = [[palette.entities.npcs.guards.m_guard_245]],  
              EyesColor = 7,  
              FeetColor = 3,  
              FeetModel = 6710062,  
              GabaritArmsWidth = 12,  
              GabaritBreastSize = 6,  
              GabaritHeight = 5,  
              GabaritLegsWidth = 5,  
              GabaritTorsoWidth = 4,  
              HairColor = 3,  
              HairType = 6711086,  
              HandsColor = 3,  
              HandsModel = 6710574,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6712622,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 5,  
              MorphTarget2 = 7,  
              MorphTarget3 = 7,  
              MorphTarget4 = 6,  
              MorphTarget5 = 5,  
              MorphTarget6 = 2,  
              MorphTarget7 = 2,  
              MorphTarget8 = 7,  
              Name = [[Varo]],  
              NoRespawn = 1,  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_guard_melee_tank_pierce_f4.creature]],  
              SheetClient = [[basic_matis_female.creature]],  
              Speed = 1,  
              Tattoo = 15,  
              TrouserColor = 3,  
              TrouserModel = 6711598,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6761006,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2705]],  
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
                InstanceId = [[Client1_2709]],  
                Class = [[Position]],  
                x = 36246.3125,  
                y = -1087.78125,  
                z = -22.75
              }
            },  
            {
              InstanceId = [[Client1_2712]],  
              Class = [[NpcCustom]],  
              Angle = -0.0625,  
              ArmColor = 3,  
              ArmModel = 6712110,  
              Base = [[palette.entities.npcs.guards.m_guard_245]],  
              EyesColor = 1,  
              FeetColor = 3,  
              FeetModel = 6710062,  
              GabaritArmsWidth = 12,  
              GabaritBreastSize = 0,  
              GabaritHeight = 0,  
              GabaritLegsWidth = 3,  
              GabaritTorsoWidth = 2,  
              HairColor = 3,  
              HairType = 6711086,  
              HandsColor = 3,  
              HandsModel = 6710574,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6712622,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 1,  
              MorphTarget2 = 5,  
              MorphTarget3 = 6,  
              MorphTarget4 = 7,  
              MorphTarget5 = 7,  
              MorphTarget6 = 4,  
              MorphTarget7 = 1,  
              MorphTarget8 = 1,  
              Name = [[Aniccio]],  
              NoRespawn = 1,  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_guard_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_matis_male.creature]],  
              Speed = 1,  
              Tattoo = 26,  
              TrouserColor = 3,  
              TrouserModel = 6711598,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6762030,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2710]],  
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
                InstanceId = [[Client1_2714]],  
                Class = [[Position]],  
                x = 36244.625,  
                y = -1087.234375,  
                z = -23
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_2703]],  
            Class = [[Position]],  
            x = 0,  
            y = 0,  
            z = 0
          }
        },  
        {
          InstanceId = [[Client1_2725]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Zorai Group]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_2723]],  
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
              InstanceId = [[Client1_2717]],  
              Class = [[NpcCustom]],  
              Angle = -0.0625,  
              ArmColor = 2,  
              ArmModel = 6733870,  
              Base = [[palette.entities.npcs.guards.z_guard_245]],  
              EyesColor = 1,  
              FeetColor = 2,  
              FeetModel = 6731822,  
              GabaritArmsWidth = 0,  
              GabaritBreastSize = 13,  
              GabaritHeight = 6,  
              GabaritLegsWidth = 13,  
              GabaritTorsoWidth = 0,  
              HairColor = 2,  
              HairType = 6732846,  
              HandsColor = 2,  
              HandsModel = 6732334,  
              InheritPos = 1,  
              JacketColor = 2,  
              JacketModel = 6734382,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 5,  
              MorphTarget2 = 7,  
              MorphTarget3 = 1,  
              MorphTarget4 = 2,  
              MorphTarget5 = 5,  
              MorphTarget6 = 3,  
              MorphTarget7 = 6,  
              MorphTarget8 = 4,  
              Name = [[Tao]],  
              NoRespawn = 1,  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_guard_melee_tank_pierce_f4.creature]],  
              SheetClient = [[basic_zorai_female.creature]],  
              Speed = 1,  
              Tattoo = 20,  
              TrouserColor = 2,  
              TrouserModel = 6733358,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6770990,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2715]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_3080]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                    }
                  },  
                  {
                    InstanceId = [[Client1_3081]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3221]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3141]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3245]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3246]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3055]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3554]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3555]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3421]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3782]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3796]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3751]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3881]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3882]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3044]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_4159]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_4160]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_4069]]),  
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
                InstanceId = [[Client1_2718]],  
                Class = [[Position]],  
                x = 36256.15625,  
                y = -1096.828125,  
                z = -17.703125
              }
            },  
            {
              InstanceId = [[Client1_2721]],  
              Class = [[NpcCustom]],  
              Angle = -0.0625,  
              ArmColor = 2,  
              ArmModel = 6733870,  
              Base = [[palette.entities.npcs.guards.z_guard_245]],  
              EyesColor = 4,  
              FeetColor = 2,  
              FeetModel = 6731822,  
              GabaritArmsWidth = 8,  
              GabaritBreastSize = 4,  
              GabaritHeight = 5,  
              GabaritLegsWidth = 14,  
              GabaritTorsoWidth = 1,  
              HairColor = 2,  
              HairType = 6732846,  
              HandsColor = 2,  
              HandsModel = 6732334,  
              InheritPos = 1,  
              JacketColor = 2,  
              JacketModel = 6734382,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 5,  
              MorphTarget2 = 7,  
              MorphTarget3 = 4,  
              MorphTarget4 = 2,  
              MorphTarget5 = 6,  
              MorphTarget6 = 6,  
              MorphTarget7 = 3,  
              MorphTarget8 = 3,  
              Name = [[Dai-Fan]],  
              NoRespawn = 1,  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_guard_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_zorai_male.creature]],  
              Speed = 1,  
              Tattoo = 9,  
              TrouserColor = 2,  
              TrouserModel = 6733358,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6772014,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2719]],  
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
                InstanceId = [[Client1_2722]],  
                Class = [[Position]],  
                x = 36252.5,  
                y = -1095.40625,  
                z = -18.796875
              }
            },  
            {
              InstanceId = [[Client1_2728]],  
              Class = [[NpcCustom]],  
              Angle = -0.0625,  
              ArmColor = 2,  
              ArmModel = 6733870,  
              Base = [[palette.entities.npcs.guards.z_guard_245]],  
              EyesColor = 2,  
              FeetColor = 2,  
              FeetModel = 6731822,  
              GabaritArmsWidth = 11,  
              GabaritBreastSize = 3,  
              GabaritHeight = 3,  
              GabaritLegsWidth = 14,  
              GabaritTorsoWidth = 5,  
              HairColor = 2,  
              HairType = 6732846,  
              HandsColor = 2,  
              HandsModel = 6732334,  
              InheritPos = 1,  
              JacketColor = 2,  
              JacketModel = 6734382,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 7,  
              MorphTarget2 = 1,  
              MorphTarget3 = 7,  
              MorphTarget4 = 0,  
              MorphTarget5 = 2,  
              MorphTarget6 = 0,  
              MorphTarget7 = 3,  
              MorphTarget8 = 0,  
              Name = [[Kungi]],  
              NoRespawn = 1,  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_guard_melee_tank_blunt_f4.creature]],  
              SheetClient = [[basic_zorai_male.creature]],  
              Speed = 1,  
              Tattoo = 24,  
              TrouserColor = 2,  
              TrouserModel = 6733358,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6770478,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2726]],  
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
                InstanceId = [[Client1_2730]],  
                Class = [[Position]],  
                x = 36254.39063,  
                y = -1096.140625,  
                z = -18.25
              }
            },  
            {
              InstanceId = [[Client1_2733]],  
              Class = [[NpcCustom]],  
              Angle = -0.0625,  
              ArmColor = 2,  
              ArmModel = 6733870,  
              Base = [[palette.entities.npcs.guards.z_guard_245]],  
              EyesColor = 0,  
              FeetColor = 2,  
              FeetModel = 6731822,  
              GabaritArmsWidth = 14,  
              GabaritBreastSize = 9,  
              GabaritHeight = 3,  
              GabaritLegsWidth = 5,  
              GabaritTorsoWidth = 4,  
              HairColor = 2,  
              HairType = 6732846,  
              HandsColor = 2,  
              HandsModel = 6732334,  
              InheritPos = 1,  
              JacketColor = 2,  
              JacketModel = 6734382,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 3,  
              MorphTarget2 = 3,  
              MorphTarget3 = 1,  
              MorphTarget4 = 4,  
              MorphTarget5 = 0,  
              MorphTarget6 = 7,  
              MorphTarget7 = 3,  
              MorphTarget8 = 7,  
              Name = [[La-Viang]],  
              NoRespawn = 1,  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_guard_melee_tank_pierce_f4.creature]],  
              SheetClient = [[basic_zorai_female.creature]],  
              Speed = 1,  
              Tattoo = 14,  
              TrouserColor = 2,  
              TrouserModel = 6733358,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6770990,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2731]],  
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
                InstanceId = [[Client1_2735]],  
                Class = [[Position]],  
                x = 36250.39063,  
                y = -1095.09375,  
                z = -19.203125
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_2724]],  
            Class = [[Position]],  
            x = -7.046875,  
            y = 3.25,  
            z = -3.671875
          }
        },  
        {
          InstanceId = [[Client1_2746]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Tryker Group]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_2744]],  
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
              InstanceId = [[Client1_2738]],  
              Class = [[NpcCustom]],  
              Angle = -0.0625,  
              ArmColor = 3,  
              ArmModel = 6722862,  
              Base = [[palette.entities.npcs.guards.t_guard_245]],  
              EyesColor = 2,  
              FeetColor = 3,  
              FeetModel = 6720814,  
              GabaritArmsWidth = 13,  
              GabaritBreastSize = 3,  
              GabaritHeight = 11,  
              GabaritLegsWidth = 3,  
              GabaritTorsoWidth = 3,  
              HairColor = 3,  
              HairType = 6721838,  
              HandsColor = 3,  
              HandsModel = 6721326,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6723374,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 5,  
              MorphTarget2 = 4,  
              MorphTarget3 = 2,  
              MorphTarget4 = 4,  
              MorphTarget5 = 2,  
              MorphTarget6 = 4,  
              MorphTarget7 = 3,  
              MorphTarget8 = 6,  
              Name = [[Ba'Dardan]],  
              NoRespawn = 1,  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_guard_melee_tank_blunt_f4.creature]],  
              SheetClient = [[basic_tryker_male.creature]],  
              Speed = 1,  
              Tattoo = 19,  
              TrouserColor = 3,  
              TrouserModel = 6722350,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6765358,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2736]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_3089]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                    }
                  },  
                  {
                    InstanceId = [[Client1_3090]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3091]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3159]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3243]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3244]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3055]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3559]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3560]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3529]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3800]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3801]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3679]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_3879]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_3880]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Follow Route]],  
                        ActivityZoneId = r2.RefId([[Client1_3044]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[No Limit]],  
                        TimeLimitValue = [[0]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      }
                    }
                  },  
                  {
                    InstanceId = [[Client1_4163]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 0,  
                    Components = {
                      {
                        InstanceId = [[Client1_4164]],  
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
                InstanceId = [[Client1_2739]],  
                Class = [[Position]],  
                x = 36258.03125,  
                y = -1091.828125,  
                z = -17.875
              }
            },  
            {
              InstanceId = [[Client1_2742]],  
              Class = [[NpcCustom]],  
              Angle = -0.0625,  
              ArmColor = 3,  
              ArmModel = 6722862,  
              Base = [[palette.entities.npcs.guards.t_guard_245]],  
              EyesColor = 5,  
              FeetColor = 3,  
              FeetModel = 6720814,  
              GabaritArmsWidth = 0,  
              GabaritBreastSize = 6,  
              GabaritHeight = 10,  
              GabaritLegsWidth = 13,  
              GabaritTorsoWidth = 0,  
              HairColor = 3,  
              HairType = 6721838,  
              HandsColor = 3,  
              HandsModel = 6721326,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6723374,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 6,  
              MorphTarget2 = 3,  
              MorphTarget3 = 4,  
              MorphTarget4 = 2,  
              MorphTarget5 = 4,  
              MorphTarget6 = 1,  
              MorphTarget7 = 7,  
              MorphTarget8 = 5,  
              Name = [[Be'Ledacan]],  
              NoRespawn = 1,  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_guard_melee_tank_pierce_f4.creature]],  
              SheetClient = [[basic_tryker_male.creature]],  
              Speed = 1,  
              Tattoo = 5,  
              TrouserColor = 3,  
              TrouserModel = 6722350,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6766126,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2740]],  
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
                InstanceId = [[Client1_2743]],  
                Class = [[Position]],  
                x = 36252.71875,  
                y = -1090.109375,  
                z = -19.46875
              }
            },  
            {
              InstanceId = [[Client1_2749]],  
              Class = [[NpcCustom]],  
              Angle = -0.0625,  
              ArmColor = 3,  
              ArmModel = 6722862,  
              Base = [[palette.entities.npcs.guards.t_guard_245]],  
              EyesColor = 3,  
              FeetColor = 3,  
              FeetModel = 6720814,  
              GabaritArmsWidth = 11,  
              GabaritBreastSize = 6,  
              GabaritHeight = 1,  
              GabaritLegsWidth = 7,  
              GabaritTorsoWidth = 5,  
              HairColor = 3,  
              HairType = 6721838,  
              HandsColor = 3,  
              HandsModel = 6721326,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6723374,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 7,  
              MorphTarget2 = 6,  
              MorphTarget3 = 5,  
              MorphTarget4 = 7,  
              MorphTarget5 = 7,  
              MorphTarget6 = 3,  
              MorphTarget7 = 7,  
              MorphTarget8 = 1,  
              Name = [[Rosira]],  
              NoRespawn = 1,  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_guard_melee_tank_blunt_f4.creature]],  
              SheetClient = [[basic_tryker_female.creature]],  
              Speed = 1,  
              Tattoo = 2,  
              TrouserColor = 3,  
              TrouserModel = 6722350,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6765358,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2747]],  
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
                InstanceId = [[Client1_2751]],  
                Class = [[Position]],  
                x = 36256.01563,  
                y = -1091.359375,  
                z = -19.171875
              }
            },  
            {
              InstanceId = [[Client1_2754]],  
              Class = [[NpcCustom]],  
              Angle = -0.0625,  
              ArmColor = 3,  
              ArmModel = 6722862,  
              Base = [[palette.entities.npcs.guards.t_guard_245]],  
              EyesColor = 0,  
              FeetColor = 3,  
              FeetModel = 6720814,  
              GabaritArmsWidth = 1,  
              GabaritBreastSize = 9,  
              GabaritHeight = 1,  
              GabaritLegsWidth = 8,  
              GabaritTorsoWidth = 5,  
              HairColor = 3,  
              HairType = 6721838,  
              HandsColor = 3,  
              HandsModel = 6721326,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6723374,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 4,  
              MorphTarget2 = 0,  
              MorphTarget3 = 1,  
              MorphTarget4 = 5,  
              MorphTarget5 = 0,  
              MorphTarget6 = 0,  
              MorphTarget7 = 6,  
              MorphTarget8 = 0,  
              Name = [[Be'Ledacan]],  
              NoRespawn = 1,  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_guard_melee_tank_pierce_f4.creature]],  
              SheetClient = [[basic_tryker_female.creature]],  
              Speed = 1,  
              Tattoo = 28,  
              TrouserColor = 3,  
              TrouserModel = 6722350,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6765870,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2752]],  
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
                InstanceId = [[Client1_2756]],  
                Class = [[Position]],  
                x = 36254.45313,  
                y = -1090.859375,  
                z = -19.296875
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_2745]],  
            Class = [[Position]],  
            x = -7.4375,  
            y = 5.15625,  
            z = -3.21875
          }
        },  
        {
          InstanceId = [[Client1_2759]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 1,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Act 3 start]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_2757]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_2760]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 5,  
              Actions = {
                {
                  InstanceId = [[Client1_2761]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_2764]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            },  
            {
              InstanceId = [[Client1_2762]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_2763]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_2767]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            },  
            {
              InstanceId = [[Client1_2765]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_2766]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_2768]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_2758]],  
            Class = [[Position]],  
            x = 36237.73438,  
            y = -1081.703125,  
            z = -22.671875
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_2774]],  
          Class = [[TalkTo]],  
          Active = 1,  
          Base = [[palette.entities.botobjects.bot_chat]],  
          ContextualText = [[Accept <mission_target>'s plan]],  
          InheritPos = 1,  
          MissionGiver = r2.RefId([[Client1_2668]]),  
          MissionSucceedText = [[I am glad you agree with me, you are a wise leader.  Lets move out!]],  
          MissionTarget = r2.RefId([[Client1_2660]]),  
          MissionText = [[Greetings. The bandits have a camp in the South East, we need to take them out before we can move on, but my Lieutenants have differing ideas about how to do this. You have proved yourself to us, so you can decide which plan is best.]],  
          Name = [[Mission: Talk to Aby]],  
          Repeatable = 0,  
          WaitValidationText = [[Sir, I think we should attack with all our units from the North.  They shall be overwhelmed by our mighty force before they know whats going on!]],  
          _Seed = 1154347415,  
          Behavior = {
            InstanceId = [[Client1_2775]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_3093]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_3096]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2683]]),  
                    Action = {
                      InstanceId = [[Client1_3095]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3084]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3098]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2704]]),  
                    Action = {
                      InstanceId = [[Client1_3097]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3087]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3100]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2725]]),  
                    Action = {
                      InstanceId = [[Client1_3099]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3081]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3102]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2746]]),  
                    Action = {
                      InstanceId = [[Client1_3101]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3090]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3104]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2664]]),  
                    Action = {
                      InstanceId = [[Client1_3103]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3078]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3106]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2668]]),  
                    Action = {
                      InstanceId = [[Client1_3105]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3069]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3108]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2660]]),  
                    Action = {
                      InstanceId = [[Client1_3107]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3072]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3110]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2771]]),  
                    Action = {
                      InstanceId = [[Client1_3109]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3075]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3112]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2777]]),  
                    Action = {
                      InstanceId = [[Client1_3111]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3114]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2780]]),  
                    Action = {
                      InstanceId = [[Client1_3113]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_3094]],  
                  Class = [[EventType]],  
                  Type = [[succeeded]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
          },  
          Position = {
            InstanceId = [[Client1_2776]],  
            Class = [[Position]],  
            x = 36267.59375,  
            y = -1104.71875,  
            z = -12.25
          }
        },  
        {
          InstanceId = [[Client1_2777]],  
          Class = [[TalkTo]],  
          Active = 1,  
          Base = [[palette.entities.botobjects.bot_chat]],  
          ContextualText = [[Accept <mission_target>'s plan]],  
          InheritPos = 1,  
          MissionGiver = r2.RefId([[Client1_2668]]),  
          MissionSucceedText = [[Good choice sir, we shall win for sure now.  Let us set up for our strike!]],  
          MissionTarget = r2.RefId([[Client1_2771]]),  
          MissionText = [[]],  
          Name = [[Mission: Talk to Anisti]],  
          Repeatable = 0,  
          WaitValidationText = [[My plan?  Well naturally dividing our enemies forces is the most sound idea.  We should attack from both sides at once, outflanking our enemy before the battle has even started.  It is a sure victory for our side!]],  
          _Seed = 1154347903,  
          Behavior = {
            InstanceId = [[Client1_2778]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_3826]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_3829]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2774]]),  
                    Action = {
                      InstanceId = [[Client1_3828]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3831]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2780]]),  
                    Action = {
                      InstanceId = [[Client1_3830]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3833]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2704]]),  
                    Action = {
                      InstanceId = [[Client1_3832]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3798]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3835]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2725]]),  
                    Action = {
                      InstanceId = [[Client1_3834]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3782]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3837]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2668]]),  
                    Action = {
                      InstanceId = [[Client1_3836]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3771]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3839]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2771]]),  
                    Action = {
                      InstanceId = [[Client1_3838]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4422]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3841]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2664]]),  
                    Action = {
                      InstanceId = [[Client1_3840]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3078]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3843]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2660]]),  
                    Action = {
                      InstanceId = [[Client1_3842]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3072]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3845]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2746]]),  
                    Action = {
                      InstanceId = [[Client1_3844]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3090]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3847]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2683]]),  
                    Action = {
                      InstanceId = [[Client1_3846]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3084]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_3827]],  
                  Class = [[EventType]],  
                  Type = [[succeeded]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
          },  
          Position = {
            InstanceId = [[Client1_2779]],  
            Class = [[Position]],  
            x = 36265.59375,  
            y = -1104.6875,  
            z = -12.046875
          }
        },  
        {
          InstanceId = [[Client1_2780]],  
          Class = [[TalkTo]],  
          Active = 1,  
          Base = [[palette.entities.botobjects.bot_chat]],  
          ContextualText = [[Accept <mission_target>'s plan]],  
          InheritPos = 1,  
          MissionGiver = r2.RefId([[Client1_2668]]),  
          MissionSucceedText = [[Very well, lets get started!]],  
          MissionTarget = r2.RefId([[Client1_2664]]),  
          MissionText = [[]],  
          Name = [[Mission: Talk to Gale]],  
          Repeatable = 0,  
          WaitValidationText = [[Hi there!  Attacking from the South would be the best idea in my opinion.  We will have more room to manuver there, and an attack in force will give us the upper hand!]],  
          _Seed = 1154348187,  
          Behavior = {
            InstanceId = [[Client1_2781]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_3803]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_3806]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2777]]),  
                    Action = {
                      InstanceId = [[Client1_3805]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3808]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2774]]),  
                    Action = {
                      InstanceId = [[Client1_3807]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3810]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2664]]),  
                    Action = {
                      InstanceId = [[Client1_3809]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3780]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3812]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2668]]),  
                    Action = {
                      InstanceId = [[Client1_3811]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3771]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3814]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2660]]),  
                    Action = {
                      InstanceId = [[Client1_3813]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3773]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3816]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2771]]),  
                    Action = {
                      InstanceId = [[Client1_3815]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3778]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3818]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2683]]),  
                    Action = {
                      InstanceId = [[Client1_3817]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3784]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3820]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2704]]),  
                    Action = {
                      InstanceId = [[Client1_3819]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3798]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3822]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2725]]),  
                    Action = {
                      InstanceId = [[Client1_3821]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3782]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3824]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2746]]),  
                    Action = {
                      InstanceId = [[Client1_3823]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3800]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_3804]],  
                  Class = [[EventType]],  
                  Type = [[succeeded]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
          },  
          Position = {
            InstanceId = [[Client1_2782]],  
            Class = [[Position]],  
            x = 36263.5,  
            y = -1105.078125,  
            z = -11.859375
          }
        },  
        {
          InstanceId = [[Client1_2894]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Bandits]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_2892]],  
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
              InstanceId = [[Client1_2886]],  
              Class = [[NpcCustom]],  
              Angle = 0.140625,  
              ArmColor = 1,  
              ArmModel = 6706478,  
              Base = [[palette.entities.npcs.bandits.f_melee_dd_220]],  
              EyesColor = 0,  
              FeetColor = 4,  
              FeetModel = 6699310,  
              GabaritArmsWidth = 6,  
              GabaritBreastSize = 8,  
              GabaritHeight = 4,  
              GabaritLegsWidth = 12,  
              GabaritTorsoWidth = 7,  
              HairColor = 3,  
              HairType = 5621806,  
              HandsColor = 2,  
              HandsModel = 6705454,  
              InheritPos = 1,  
              JacketColor = 1,  
              JacketModel = 6706990,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 3,  
              MorphTarget2 = 1,  
              MorphTarget3 = 7,  
              MorphTarget4 = 5,  
              MorphTarget5 = 3,  
              MorphTarget6 = 0,  
              MorphTarget7 = 5,  
              MorphTarget8 = 7,  
              Name = [[Aecaon]],  
              NoRespawn = 1,  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_melee_damage_dealer_slash_f4.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 1,  
              Tattoo = 16,  
              TrouserColor = 1,  
              TrouserModel = 6706222,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6756398,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2884]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_2983]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_2986]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_2964]]),  
                        Action = {
                          InstanceId = [[Client1_2985]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                    },  
                    Event = {
                      InstanceId = [[Client1_2984]],  
                      Class = [[EventType]],  
                      Type = [[group death]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_2956]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_2958]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Wander]],  
                        ActivityZoneId = r2.RefId([[Client1_2871]]),  
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
                InstanceId = [[Client1_2887]],  
                Class = [[Position]],  
                x = 36441.75,  
                y = -1211.796875,  
                z = -19.03125
              }
            },  
            {
              InstanceId = [[Client1_2890]],  
              Class = [[NpcCustom]],  
              Angle = 0.140625,  
              ArmColor = 4,  
              ArmModel = 0,  
              Base = [[palette.entities.npcs.bandits.f_melee_dd_220]],  
              EyesColor = 1,  
              FeetColor = 5,  
              FeetModel = 6705198,  
              GabaritArmsWidth = 10,  
              GabaritBreastSize = 3,  
              GabaritHeight = 10,  
              GabaritLegsWidth = 11,  
              GabaritTorsoWidth = 0,  
              HairColor = 4,  
              HairType = 5621806,  
              HandsColor = 2,  
              HandsModel = 6705454,  
              InheritPos = 1,  
              JacketColor = 5,  
              JacketModel = 6707246,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 5,  
              MorphTarget2 = 4,  
              MorphTarget3 = 1,  
              MorphTarget4 = 1,  
              MorphTarget5 = 5,  
              MorphTarget6 = 4,  
              MorphTarget7 = 5,  
              MorphTarget8 = 1,  
              Name = [[Xan]],  
              NoRespawn = 1,  
              Sex = 1,  
              Sheet = [[ring_melee_damage_dealer_blunt_f4.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 1,  
              Tattoo = 30,  
              TrouserColor = 3,  
              TrouserModel = 6706222,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6755374,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2888]],  
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
                InstanceId = [[Client1_2891]],  
                Class = [[Position]],  
                x = 36441.85938,  
                y = -1210.046875,  
                z = -18.828125
              }
            },  
            {
              InstanceId = [[Client1_2897]],  
              Class = [[NpcCustom]],  
              Angle = 0.140625,  
              ArmColor = 0,  
              ArmModel = 6704174,  
              Base = [[palette.entities.npcs.bandits.f_mage_aoe_220]],  
              EyesColor = 4,  
              FeetColor = 4,  
              FeetModel = 6702638,  
              GabaritArmsWidth = 6,  
              GabaritBreastSize = 14,  
              GabaritHeight = 6,  
              GabaritLegsWidth = 8,  
              GabaritTorsoWidth = 3,  
              HairColor = 1,  
              HairType = 5621806,  
              HandsColor = 0,  
              HandsModel = 6703150,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6704430,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 2,  
              MorphTarget2 = 0,  
              MorphTarget3 = 7,  
              MorphTarget4 = 6,  
              MorphTarget5 = 2,  
              MorphTarget6 = 2,  
              MorphTarget7 = 6,  
              MorphTarget8 = 0,  
              Name = [[Diorius]],  
              NoRespawn = 1,  
              Sex = 0,  
              Sheet = [[ring_magic_aoe_acid_f4.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 1,  
              Tattoo = 11,  
              TrouserColor = 2,  
              TrouserModel = 6697774,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6934318,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2895]],  
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
                InstanceId = [[Client1_2899]],  
                Class = [[Position]],  
                x = 36442.84375,  
                y = -1213.328125,  
                z = -19.265625
              }
            },  
            {
              InstanceId = [[Client1_2902]],  
              Class = [[NpcCustom]],  
              Angle = 0.140625,  
              ArmColor = 3,  
              ArmModel = 6704174,  
              Base = [[palette.entities.npcs.bandits.f_mage_aoe_220]],  
              EyesColor = 2,  
              FeetColor = 3,  
              FeetModel = 6702382,  
              GabaritArmsWidth = 14,  
              GabaritBreastSize = 7,  
              GabaritHeight = 12,  
              GabaritLegsWidth = 3,  
              GabaritTorsoWidth = 7,  
              HairColor = 4,  
              HairType = 2862,  
              HandsColor = 3,  
              HandsModel = 6703150,  
              InheritPos = 1,  
              JacketColor = 0,  
              JacketModel = 6704430,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 7,  
              MorphTarget2 = 1,  
              MorphTarget3 = 1,  
              MorphTarget4 = 1,  
              MorphTarget5 = 0,  
              MorphTarget6 = 6,  
              MorphTarget7 = 5,  
              MorphTarget8 = 4,  
              Name = [[Dykos]],  
              NoRespawn = 1,  
              Sex = 0,  
              Sheet = [[ring_magic_aoe_acid_f4.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 1,  
              Tattoo = 28,  
              TrouserColor = 0,  
              TrouserModel = 6698030,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6934318,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2900]],  
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
                InstanceId = [[Client1_2904]],  
                Class = [[Position]],  
                x = 36443.39063,  
                y = -1208.828125,  
                z = -18.875
              }
            },  
            {
              InstanceId = [[Client1_2907]],  
              Class = [[NpcCustom]],  
              Angle = 0.140625,  
              ArmColor = 2,  
              ArmModel = 6704174,  
              Base = [[palette.entities.npcs.bandits.f_mage_damage_dealer_220]],  
              EyesColor = 4,  
              FeetColor = 3,  
              FeetModel = 6702638,  
              GabaritArmsWidth = 10,  
              GabaritBreastSize = 13,  
              GabaritHeight = 11,  
              GabaritLegsWidth = 0,  
              GabaritTorsoWidth = 6,  
              HairColor = 3,  
              HairType = 5622062,  
              HandsColor = 2,  
              HandsModel = 6703150,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6704430,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 4,  
              MorphTarget2 = 4,  
              MorphTarget3 = 1,  
              MorphTarget4 = 3,  
              MorphTarget5 = 0,  
              MorphTarget6 = 3,  
              MorphTarget7 = 3,  
              MorphTarget8 = 7,  
              Name = [[Zekos]],  
              NoRespawn = 1,  
              Sex = 0,  
              Sheet = [[ring_magic_damage_dealer_acid_f4.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 1,  
              Tattoo = 15,  
              TrouserColor = 4,  
              TrouserModel = 6703406,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6934318,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2905]],  
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
                InstanceId = [[Client1_2909]],  
                Class = [[Position]],  
                x = 36443.75,  
                y = -1211.265625,  
                z = -19.25
              }
            },  
            {
              InstanceId = [[Client1_2912]],  
              Class = [[NpcCustom]],  
              Angle = 0.140625,  
              ArmColor = 0,  
              ArmModel = 6704174,  
              Base = [[palette.entities.npcs.bandits.f_mage_damage_dealer_220]],  
              EyesColor = 4,  
              FeetColor = 2,  
              FeetModel = 6702638,  
              GabaritArmsWidth = 12,  
              GabaritBreastSize = 7,  
              GabaritHeight = 4,  
              GabaritLegsWidth = 10,  
              GabaritTorsoWidth = 1,  
              HairColor = 5,  
              HairType = 2350,  
              HandsColor = 3,  
              HandsModel = 6702894,  
              InheritPos = 1,  
              JacketColor = 5,  
              JacketModel = 6704430,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 3,  
              MorphTarget2 = 6,  
              MorphTarget3 = 2,  
              MorphTarget4 = 2,  
              MorphTarget5 = 1,  
              MorphTarget6 = 6,  
              MorphTarget7 = 0,  
              MorphTarget8 = 7,  
              Name = [[Zexius]],  
              NoRespawn = 1,  
              Sex = 1,  
              Sheet = [[ring_magic_damage_dealer_acid_f4.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 1,  
              Tattoo = 7,  
              TrouserColor = 4,  
              TrouserModel = 6703406,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6934318,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2910]],  
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
                InstanceId = [[Client1_2914]],  
                Class = [[Position]],  
                x = 36440.96875,  
                y = -1213.609375,  
                z = -19.03125
              }
            },  
            {
              InstanceId = [[Client1_2917]],  
              Class = [[NpcCustom]],  
              Angle = 0.140625,  
              ArmColor = 0,  
              ArmModel = 6704174,  
              Base = [[palette.entities.npcs.bandits.f_mage_celestial_curser_220]],  
              EyesColor = 7,  
              FeetColor = 0,  
              FeetModel = 6702382,  
              GabaritArmsWidth = 6,  
              GabaritBreastSize = 6,  
              GabaritHeight = 14,  
              GabaritLegsWidth = 13,  
              GabaritTorsoWidth = 3,  
              HairColor = 0,  
              HairType = 5621550,  
              HandsColor = 1,  
              HandsModel = 6703150,  
              InheritPos = 1,  
              JacketColor = 0,  
              JacketModel = 6704430,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 5,  
              MorphTarget2 = 4,  
              MorphTarget3 = 0,  
              MorphTarget4 = 1,  
              MorphTarget5 = 2,  
              MorphTarget6 = 5,  
              MorphTarget7 = 5,  
              MorphTarget8 = 4,  
              Name = [[Apolion]],  
              NoRespawn = 1,  
              Sex = 1,  
              Sheet = [[ring_magic_curser_fear_f4.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 1,  
              Tattoo = 11,  
              TrouserColor = 5,  
              TrouserModel = 6697774,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6934318,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2915]],  
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
                InstanceId = [[Client1_2919]],  
                Class = [[Position]],  
                x = 36440.46875,  
                y = -1210.953125,  
                z = -18.71875
              }
            },  
            {
              InstanceId = [[Client1_2922]],  
              Class = [[NpcCustom]],  
              Angle = 0.140625,  
              ArmColor = 4,  
              ArmModel = 6704174,  
              Base = [[palette.entities.npcs.bandits.f_light_melee_220]],  
              EyesColor = 1,  
              FeetColor = 2,  
              FeetModel = 6704942,  
              GabaritArmsWidth = 1,  
              GabaritBreastSize = 8,  
              GabaritHeight = 13,  
              GabaritLegsWidth = 4,  
              GabaritTorsoWidth = 5,  
              HairColor = 5,  
              HairType = 5621806,  
              HandsColor = 4,  
              HandsModel = 6705454,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6707246,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 1,  
              MorphTarget2 = 0,  
              MorphTarget3 = 3,  
              MorphTarget4 = 2,  
              MorphTarget5 = 4,  
              MorphTarget6 = 6,  
              MorphTarget7 = 7,  
              MorphTarget8 = 4,  
              Name = [[Iolaus]],  
              NoRespawn = 1,  
              Sex = 1,  
              Sheet = [[ring_light_melee_pierce_f4.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 1,  
              Tattoo = 25,  
              TrouserColor = 5,  
              TrouserModel = 6703662,  
              WeaponLeftHand = 6753070,  
              WeaponRightHand = 6753070,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2920]],  
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
                InstanceId = [[Client1_2924]],  
                Class = [[Position]],  
                x = 36444.14063,  
                y = -1212.4375,  
                z = -19.390625
              }
            },  
            {
              InstanceId = [[Client1_2927]],  
              Class = [[NpcCustom]],  
              Angle = 0.140625,  
              ArmColor = 2,  
              ArmModel = 6703918,  
              Base = [[palette.entities.npcs.bandits.f_mage_atysian_curser_220]],  
              EyesColor = 0,  
              FeetColor = 2,  
              FeetModel = 6702382,  
              GabaritArmsWidth = 14,  
              GabaritBreastSize = 10,  
              GabaritHeight = 0,  
              GabaritLegsWidth = 13,  
              GabaritTorsoWidth = 4,  
              HairColor = 5,  
              HairType = 5621806,  
              HandsColor = 3,  
              HandsModel = 6702894,  
              InheritPos = 1,  
              JacketColor = 0,  
              JacketModel = 6704686,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 7,  
              MorphTarget2 = 0,  
              MorphTarget3 = 6,  
              MorphTarget4 = 6,  
              MorphTarget5 = 7,  
              MorphTarget6 = 1,  
              MorphTarget7 = 1,  
              MorphTarget8 = 5,  
              Name = [[Plello]],  
              Sex = 0,  
              Sheet = [[ring_magic_curser_blind_f4.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 1,  
              Tattoo = 24,  
              TrouserColor = 0,  
              TrouserModel = 6697774,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6934318,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2925]],  
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
                InstanceId = [[Client1_2929]],  
                Class = [[Position]],  
                x = 36443.64063,  
                y = -1209.9375,  
                z = -19.078125
              }
            },  
            {
              InstanceId = [[Client1_2932]],  
              Class = [[NpcCustom]],  
              Angle = 0.140625,  
              ArmColor = 0,  
              ArmModel = 6701358,  
              Base = [[palette.entities.npcs.bandits.f_melee_tank_220]],  
              EyesColor = 1,  
              FeetColor = 5,  
              FeetModel = 6699566,  
              GabaritArmsWidth = 9,  
              GabaritBreastSize = 2,  
              GabaritHeight = 12,  
              GabaritLegsWidth = 5,  
              GabaritTorsoWidth = 4,  
              HairColor = 5,  
              HairType = 6700334,  
              HandsColor = 0,  
              HandsModel = 6699822,  
              InheritPos = 1,  
              JacketColor = 2,  
              JacketModel = 6701870,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 6,  
              MorphTarget2 = 4,  
              MorphTarget3 = 1,  
              MorphTarget4 = 0,  
              MorphTarget5 = 3,  
              MorphTarget6 = 1,  
              MorphTarget7 = 5,  
              MorphTarget8 = 0,  
              Name = [[Abytheus]],  
              NoRespawn = 1,  
              Sex = 1,  
              Sheet = [[ring_melee_tank_pierce_f4.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 1,  
              Tattoo = 28,  
              TrouserColor = 0,  
              TrouserModel = 6700846,  
              WeaponLeftHand = 6773294,  
              WeaponRightHand = 6753838,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2930]],  
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
                InstanceId = [[Client1_2934]],  
                Class = [[Position]],  
                x = 36440.39063,  
                y = -1209.296875,  
                z = -18.5
              }
            },  
            {
              InstanceId = [[Client1_2937]],  
              Class = [[NpcCustom]],  
              Angle = 0.140625,  
              ArmColor = 4,  
              ArmModel = 6701358,  
              Base = [[palette.entities.npcs.bandits.f_melee_tank_220]],  
              EyesColor = 3,  
              FeetColor = 0,  
              FeetModel = 6699566,  
              GabaritArmsWidth = 10,  
              GabaritBreastSize = 5,  
              GabaritHeight = 7,  
              GabaritLegsWidth = 0,  
              GabaritTorsoWidth = 1,  
              HairColor = 2,  
              HairType = 6700590,  
              HandsColor = 3,  
              HandsModel = 6700078,  
              InheritPos = 1,  
              JacketColor = 2,  
              JacketModel = 6701870,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 7,  
              MorphTarget2 = 2,  
              MorphTarget3 = 1,  
              MorphTarget4 = 0,  
              MorphTarget5 = 2,  
              MorphTarget6 = 7,  
              MorphTarget7 = 5,  
              MorphTarget8 = 0,  
              Name = [[Pleps]],  
              NoRespawn = 1,  
              Sex = 0,  
              Sheet = [[ring_melee_tank_pierce_f4.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 1,  
              Tattoo = 29,  
              TrouserColor = 1,  
              TrouserModel = 6700846,  
              WeaponLeftHand = 6773038,  
              WeaponRightHand = 6753582,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2935]],  
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
                InstanceId = [[Client1_2939]],  
                Class = [[Position]],  
                x = 36441.6875,  
                y = -1214.546875,  
                z = -19.171875
              }
            },  
            {
              InstanceId = [[Client1_2942]],  
              Class = [[NpcCustom]],  
              Angle = 0.140625,  
              ArmColor = 2,  
              ArmModel = 6703918,  
              Base = [[palette.entities.npcs.bandits.f_light_melee_220]],  
              EyesColor = 0,  
              FeetColor = 5,  
              FeetModel = 6702382,  
              GabaritArmsWidth = 5,  
              GabaritBreastSize = 4,  
              GabaritHeight = 14,  
              GabaritLegsWidth = 4,  
              GabaritTorsoWidth = 4,  
              HairColor = 0,  
              HairType = 5621806,  
              HandsColor = 5,  
              HandsModel = 6705710,  
              InheritPos = 1,  
              JacketColor = 4,  
              JacketModel = 6707246,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 3,  
              MorphTarget2 = 4,  
              MorphTarget3 = 4,  
              MorphTarget4 = 6,  
              MorphTarget5 = 3,  
              MorphTarget6 = 0,  
              MorphTarget7 = 2,  
              MorphTarget8 = 4,  
              Name = [[Pilion]],  
              NoRespawn = 1,  
              Sex = 0,  
              Sheet = [[ring_light_melee_slash_f4.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 1,  
              Tattoo = 23,  
              TrouserColor = 0,  
              TrouserModel = 6706222,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6754606,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_2940]],  
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
                InstanceId = [[Client1_2944]],  
                Class = [[Position]],  
                x = 36440.21875,  
                y = -1212.609375,  
                z = -18.84375
              }
            },  
            {
              InstanceId = [[Client1_4435]],  
              Class = [[NpcCustom]],  
              Angle = -0.765625,  
              ArmColor = 0,  
              ArmModel = 0,  
              AutoSpawn = 0,  
              Base = [[palette.entities.npcs.bandits.f_light_melee_220]],  
              EyesColor = 4,  
              FeetColor = 5,  
              FeetModel = 6702638,  
              GabaritArmsWidth = 5,  
              GabaritBreastSize = 9,  
              GabaritHeight = 4,  
              GabaritLegsWidth = 13,  
              GabaritTorsoWidth = 7,  
              HairColor = 3,  
              HairType = 5622062,  
              HandsColor = 4,  
              HandsModel = 6705710,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6704430,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 1,  
              MorphTarget2 = 0,  
              MorphTarget3 = 1,  
              MorphTarget4 = 6,  
              MorphTarget5 = 2,  
              MorphTarget6 = 3,  
              MorphTarget7 = 6,  
              MorphTarget8 = 5,  
              Name = [[Diops]],  
              NoRespawn = 1,  
              Sex = 0,  
              Sheet = [[ring_light_melee_pierce_f4.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 1,  
              Tattoo = 7,  
              TrouserColor = 5,  
              TrouserModel = 6703662,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6753582,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4433]],  
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
                InstanceId = [[Client1_4437]],  
                Class = [[Position]],  
                x = 36442.34375,  
                y = -1213.96875,  
                z = -19.234375
              }
            },  
            {
              InstanceId = [[Client1_4440]],  
              Class = [[NpcCustom]],  
              Angle = -0.765625,  
              ArmColor = 2,  
              ArmModel = 6706734,  
              AutoSpawn = 0,  
              Base = [[palette.entities.npcs.bandits.f_melee_dd_220]],  
              EyesColor = 4,  
              FeetColor = 4,  
              FeetModel = 6699310,  
              GabaritArmsWidth = 4,  
              GabaritBreastSize = 8,  
              GabaritHeight = 1,  
              GabaritLegsWidth = 6,  
              GabaritTorsoWidth = 6,  
              HairColor = 2,  
              HairType = 2606,  
              HandsColor = 3,  
              HandsModel = 6705710,  
              InheritPos = 1,  
              JacketColor = 0,  
              JacketModel = 6701870,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 0,  
              MorphTarget2 = 1,  
              MorphTarget3 = 7,  
              MorphTarget4 = 6,  
              MorphTarget5 = 4,  
              MorphTarget6 = 2,  
              MorphTarget7 = 1,  
              MorphTarget8 = 4,  
              Name = [[Aekos]],  
              NoRespawn = 1,  
              Sex = 1,  
              Sheet = [[ring_melee_damage_dealer_slash_f4.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 1,  
              Tattoo = 20,  
              TrouserColor = 4,  
              TrouserModel = 6705966,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6756654,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4438]],  
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
                InstanceId = [[Client1_4442]],  
                Class = [[Position]],  
                x = 36443.39063,  
                y = -1212.765625,  
                z = -19.3125
              }
            },  
            {
              InstanceId = [[Client1_4445]],  
              Class = [[NpcCustom]],  
              Angle = -0.765625,  
              ArmColor = 3,  
              ArmModel = 6706478,  
              AutoSpawn = 0,  
              Base = [[palette.entities.npcs.bandits.f_melee_dd_220]],  
              EyesColor = 1,  
              FeetColor = 0,  
              FeetModel = 6704942,  
              GabaritArmsWidth = 13,  
              GabaritBreastSize = 0,  
              GabaritHeight = 14,  
              GabaritLegsWidth = 10,  
              GabaritTorsoWidth = 5,  
              HairColor = 0,  
              HairType = 2862,  
              HandsColor = 0,  
              HandsModel = 6705454,  
              InheritPos = 1,  
              JacketColor = 0,  
              JacketModel = 6702126,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 1,  
              MorphTarget2 = 7,  
              MorphTarget3 = 7,  
              MorphTarget4 = 6,  
              MorphTarget5 = 0,  
              MorphTarget6 = 3,  
              MorphTarget7 = 3,  
              MorphTarget8 = 5,  
              Name = [[Piion]],  
              NoRespawn = 1,  
              Sex = 0,  
              Sheet = [[ring_melee_damage_dealer_slash_f4.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 1,  
              Tattoo = 28,  
              TrouserColor = 2,  
              TrouserModel = 6706222,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6756398,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4443]],  
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
                InstanceId = [[Client1_4447]],  
                Class = [[Position]],  
                x = 36442.60938,  
                y = -1209.921875,  
                z = -18.921875
              }
            },  
            {
              InstanceId = [[Client1_4450]],  
              Class = [[NpcCustom]],  
              Angle = -0.765625,  
              ArmColor = 5,  
              ArmModel = 6706478,  
              AutoSpawn = 0,  
              Base = [[palette.entities.npcs.bandits.f_melee_dd_220]],  
              EyesColor = 1,  
              FeetColor = 2,  
              FeetModel = 6705198,  
              GabaritArmsWidth = 5,  
              GabaritBreastSize = 5,  
              GabaritHeight = 6,  
              GabaritLegsWidth = 5,  
              GabaritTorsoWidth = 2,  
              HairColor = 2,  
              HairType = 2862,  
              HandsColor = 4,  
              HandsModel = 6705454,  
              InheritPos = 1,  
              JacketColor = 2,  
              JacketModel = 6701870,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 6,  
              MorphTarget2 = 0,  
              MorphTarget3 = 2,  
              MorphTarget4 = 2,  
              MorphTarget5 = 6,  
              MorphTarget6 = 5,  
              MorphTarget7 = 5,  
              MorphTarget8 = 4,  
              Name = [[Boeion]],  
              NoRespawn = 1,  
              Sex = 1,  
              Sheet = [[ring_melee_damage_dealer_slash_f4.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 1,  
              Tattoo = 31,  
              TrouserColor = 4,  
              TrouserModel = 6705966,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6756398,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4448]],  
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
                InstanceId = [[Client1_4452]],  
                Class = [[Position]],  
                x = 36440.35938,  
                y = -1211.890625,  
                z = -18.8125
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_2893]],  
            Class = [[Position]],  
            x = 0,  
            y = 0,  
            z = 0
          }
        },  
        {
          InstanceId = [[Client1_2964]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Act 3 finish]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_2962]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_2978]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_2981]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2971]]),  
                    Action = {
                      InstanceId = [[Client1_2980]],  
                      Class = [[ActionType]],  
                      Type = [[Activate]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_2979]],  
                  Class = [[EventType]],  
                  Type = [[end of dialog]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_2965]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_2966]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_2969]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            },  
            {
              InstanceId = [[Client1_2967]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_2968]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_2970]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_2963]],  
            Class = [[Position]],  
            x = 36442.48438,  
            y = -1189.984375,  
            z = -15
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_2971]],  
          Class = [[Timer]],  
          Active = 0,  
          Base = [[palette.entities.botobjects.timer]],  
          Cyclic = 0,  
          InheritPos = 1,  
          Minutes = 0,  
          Name = [[End act 3 timer]],  
          Secondes = 6,  
          Behavior = {
            InstanceId = [[Client1_2972]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_4794]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_4797]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_4558]]),  
                    Action = {
                      InstanceId = [[Client1_4796]],  
                      Class = [[ActionType]],  
                      Type = [[Start Act]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_4795]],  
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
            InstanceId = [[Client1_2973]],  
            Class = [[Position]],  
            x = 36446,  
            y = -1190.40625,  
            z = -15
          }
        },  
        {
          InstanceId = [[Client1_3222]],  
          Class = [[TalkTo]],  
          Active = 0,  
          Base = [[palette.entities.botobjects.bot_chat]],  
          ContextualText = [[Yes lets attack!]],  
          InheritPos = 1,  
          MissionGiver = r2.RefId([[Client1_2668]]),  
          MissionSucceedText = [[CHARGE!!]],  
          MissionTarget = r2.RefId([[Client1_2660]]),  
          MissionText = [[Talk to Abyrixius if you wish to confirm the attack, or Gale or Anisti to change your mind on the plan.]],  
          Name = [[Mission: Talk to Aby attack]],  
          Repeatable = 0,  
          WaitValidationText = [[Are you ready to attack sir?]],  
          _Seed = 1154353748,  
          Behavior = {
            InstanceId = [[Client1_3223]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_3248]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_3251]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2683]]),  
                    Action = {
                      InstanceId = [[Client1_3250]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3238]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3253]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2704]]),  
                    Action = {
                      InstanceId = [[Client1_3252]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3241]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3255]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2725]]),  
                    Action = {
                      InstanceId = [[Client1_3254]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3245]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3257]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2746]]),  
                    Action = {
                      InstanceId = [[Client1_3256]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3243]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3259]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2664]]),  
                    Action = {
                      InstanceId = [[Client1_3258]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3236]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3261]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2668]]),  
                    Action = {
                      InstanceId = [[Client1_3260]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3234]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3263]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2660]]),  
                    Action = {
                      InstanceId = [[Client1_3262]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3230]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3265]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2771]]),  
                    Action = {
                      InstanceId = [[Client1_3264]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3232]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3277]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_3266]]),  
                    Action = {
                      InstanceId = [[Client1_3276]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3279]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_3269]]),  
                    Action = {
                      InstanceId = [[Client1_3278]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_3249]],  
                  Class = [[EventType]],  
                  Type = [[succeeded]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
          },  
          Position = {
            InstanceId = [[Client1_3224]],  
            Class = [[Position]],  
            x = 36265.46875,  
            y = -1085.609375,  
            z = -14.265625
          }
        },  
        {
          InstanceId = [[Client1_3266]],  
          Class = [[TalkTo]],  
          Active = 0,  
          Base = [[palette.entities.botobjects.bot_chat]],  
          ContextualText = [[Switch to <mission_target>'s plan]],  
          InheritPos = 1,  
          MissionGiver = r2.RefId([[Client1_2668]]),  
          MissionSucceedText = [[Not afraid to change your mind, I like that, lets go!]],  
          MissionTarget = r2.RefId([[Client1_2771]]),  
          MissionText = [[]],  
          Name = [[Mission: Talk to Anisti North]],  
          Repeatable = 0,  
          WaitValidationText = [[My plan?  Well naturally dividing the enemies forces is the most sound idea.  We should attack from both sides at once, outflanking our enemy before the battle has even started.  It is a sure victory for our side!]],  
          _Seed = 1154354532,  
          Behavior = {
            InstanceId = [[Client1_3267]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_3281]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_3284]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_3222]]),  
                    Action = {
                      InstanceId = [[Client1_3283]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3286]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_3269]]),  
                    Action = {
                      InstanceId = [[Client1_3285]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3578]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2704]]),  
                    Action = {
                      InstanceId = [[Client1_3577]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3557]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3580]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2725]]),  
                    Action = {
                      InstanceId = [[Client1_3579]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3554]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3582]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2668]]),  
                    Action = {
                      InstanceId = [[Client1_3581]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3546]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3584]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2771]]),  
                    Action = {
                      InstanceId = [[Client1_3583]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4309]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_3282]],  
                  Class = [[EventType]],  
                  Type = [[succeeded]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
          },  
          Position = {
            InstanceId = [[Client1_3268]],  
            Class = [[Position]],  
            x = 36265.84375,  
            y = -1083.90625,  
            z = -13.578125
          }
        },  
        {
          InstanceId = [[Client1_3269]],  
          Class = [[TalkTo]],  
          Active = 0,  
          Base = [[palette.entities.botobjects.bot_chat]],  
          ContextualText = [[Switch to <mission_target>'s plan]],  
          InheritPos = 1,  
          MissionGiver = r2.RefId([[Client1_2668]]),  
          MissionSucceedText = [[Ah glad you are switching, you had me worried there for a minute, lets go!]],  
          MissionTarget = r2.RefId([[Client1_2664]]),  
          MissionText = [[]],  
          Name = [[Mission: Talk to Gale North]],  
          Repeatable = 0,  
          WaitValidationText = [[Hi there!  Attacking from the South would be the best idea in my opinion.  We will have more room to manuver there, and an attack in force will give us the upper hand!]],  
          _Seed = 1154354776,  
          Behavior = {
            InstanceId = [[Client1_3270]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_3288]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_3291]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_3222]]),  
                    Action = {
                      InstanceId = [[Client1_3290]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3293]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_3266]]),  
                    Action = {
                      InstanceId = [[Client1_3292]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3562]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2683]]),  
                    Action = {
                      InstanceId = [[Client1_3561]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3240]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3564]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2704]]),  
                    Action = {
                      InstanceId = [[Client1_3563]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3557]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3566]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2725]]),  
                    Action = {
                      InstanceId = [[Client1_3565]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3554]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3568]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2746]]),  
                    Action = {
                      InstanceId = [[Client1_3567]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3559]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3570]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2664]]),  
                    Action = {
                      InstanceId = [[Client1_3569]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3552]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3572]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2668]]),  
                    Action = {
                      InstanceId = [[Client1_3571]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3546]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3574]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2660]]),  
                    Action = {
                      InstanceId = [[Client1_3573]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3550]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3576]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2771]]),  
                    Action = {
                      InstanceId = [[Client1_3575]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3548]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_3289]],  
                  Class = [[EventType]],  
                  Type = [[succeeded]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
          },  
          Position = {
            InstanceId = [[Client1_3271]],  
            Class = [[Position]],  
            x = 36266.01563,  
            y = -1082.765625,  
            z = -13
          }
        },  
        {
          InstanceId = [[Client1_3848]],  
          Class = [[TalkTo]],  
          Active = 0,  
          Base = [[palette.entities.botobjects.bot_chat]],  
          ContextualText = [[Yes lets attack!]],  
          InheritPos = 1,  
          MissionGiver = r2.RefId([[Client1_2668]]),  
          MissionSucceedText = [[CHARGE!!]],  
          MissionTarget = r2.RefId([[Client1_2664]]),  
          MissionText = [[Talk to Gale if you wish to confirm the attack, or Abyrixius or Anisti to change your mind on the plan.]],  
          Name = [[Mission: Talk to Gale South]],  
          Repeatable = 0,  
          WaitValidationText = [[Do you wish to attack sir?]],  
          _Seed = 1154357853,  
          Behavior = {
            InstanceId = [[Client1_3849]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_3884]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_3887]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_3851]]),  
                    Action = {
                      InstanceId = [[Client1_3886]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3889]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_3854]]),  
                    Action = {
                      InstanceId = [[Client1_3888]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3891]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2683]]),  
                    Action = {
                      InstanceId = [[Client1_3890]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3874]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3893]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2704]]),  
                    Action = {
                      InstanceId = [[Client1_3892]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3877]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3895]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2725]]),  
                    Action = {
                      InstanceId = [[Client1_3894]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3881]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3897]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2746]]),  
                    Action = {
                      InstanceId = [[Client1_3896]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3879]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3899]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2664]]),  
                    Action = {
                      InstanceId = [[Client1_3898]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3872]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3901]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2668]]),  
                    Action = {
                      InstanceId = [[Client1_3900]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3870]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3903]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2660]]),  
                    Action = {
                      InstanceId = [[Client1_3902]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3866]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_3905]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2771]]),  
                    Action = {
                      InstanceId = [[Client1_3904]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3868]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_3885]],  
                  Class = [[EventType]],  
                  Type = [[succeeded]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
          },  
          Position = {
            InstanceId = [[Client1_3850]],  
            Class = [[Position]],  
            x = 36243.34375,  
            y = -1101.890625,  
            z = -20.34375
          }
        },  
        {
          InstanceId = [[Client1_3851]],  
          Class = [[TalkTo]],  
          Active = 0,  
          Base = [[palette.entities.botobjects.bot_chat]],  
          ContextualText = [[Change to <mission_target>'s plan]],  
          InheritPos = 1,  
          MissionGiver = r2.RefId([[Client1_2668]]),  
          MissionSucceedText = [[I am glad you had the courage to change your mind.  Lets move out!]],  
          MissionTarget = r2.RefId([[Client1_2660]]),  
          MissionText = [[]],  
          Name = [[Mission: Talk to Aby South]],  
          Repeatable = 0,  
          WaitValidationText = [[Sir, I think we should attack with all our units from the North.  They shall be overwhelmed by our mighty force before they know whats going on!]],  
          _Seed = 1154357906,  
          Behavior = {
            InstanceId = [[Client1_3852]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_4207]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_4210]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_3848]]),  
                    Action = {
                      InstanceId = [[Client1_4209]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4212]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_3854]]),  
                    Action = {
                      InstanceId = [[Client1_4211]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4214]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2683]]),  
                    Action = {
                      InstanceId = [[Client1_4213]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3876]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4216]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2704]]),  
                    Action = {
                      InstanceId = [[Client1_4215]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4161]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4218]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2746]]),  
                    Action = {
                      InstanceId = [[Client1_4217]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4163]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4220]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2725]]),  
                    Action = {
                      InstanceId = [[Client1_4219]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4159]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4222]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2664]]),  
                    Action = {
                      InstanceId = [[Client1_4221]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4167]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4224]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2668]]),  
                    Action = {
                      InstanceId = [[Client1_4223]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4165]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4226]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2660]]),  
                    Action = {
                      InstanceId = [[Client1_4225]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4171]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4228]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2771]]),  
                    Action = {
                      InstanceId = [[Client1_4227]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4169]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_4208]],  
                  Class = [[EventType]],  
                  Type = [[succeeded]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
          },  
          Position = {
            InstanceId = [[Client1_3853]],  
            Class = [[Position]],  
            x = 36246.64063,  
            y = -1105.5,  
            z = -17.890625
          }
        },  
        {
          InstanceId = [[Client1_3854]],  
          Class = [[TalkTo]],  
          Active = 0,  
          Base = [[palette.entities.botobjects.bot_chat]],  
          ContextualText = [[Change to <mission_target>'s plan]],  
          InheritPos = 1,  
          MissionGiver = r2.RefId([[Client1_2668]]),  
          MissionSucceedText = [[Not afraid to change your mind, I like that, lets go!]],  
          MissionTarget = r2.RefId([[Client1_2771]]),  
          MissionText = [[]],  
          Name = [[Mission: Talk to Ani South]],  
          Repeatable = 0,  
          WaitValidationText = [[My plan?  Well naturally dividing the enemies forces is the most sound idea.  We should attack from both sides at once, outflanking the enemy before the battle has even started.  It is a sure victory for our side!]],  
          _Seed = 1154357987,  
          Behavior = {
            InstanceId = [[Client1_3855]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_4192]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_4195]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2664]]),  
                    Action = {
                      InstanceId = [[Client1_4194]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4167]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4197]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2660]]),  
                    Action = {
                      InstanceId = [[Client1_4196]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4171]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4199]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2746]]),  
                    Action = {
                      InstanceId = [[Client1_4198]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4163]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4201]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2683]]),  
                    Action = {
                      InstanceId = [[Client1_4200]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4307]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4203]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_3848]]),  
                    Action = {
                      InstanceId = [[Client1_4202]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4205]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_3851]]),  
                    Action = {
                      InstanceId = [[Client1_4204]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_4193]],  
                  Class = [[EventType]],  
                  Type = [[succeeded]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
          },  
          Position = {
            InstanceId = [[Client1_3856]],  
            Class = [[Position]],  
            x = 36244.95313,  
            y = -1104.15625,  
            z = -19
          }
        },  
        {
          InstanceId = [[Client1_4312]],  
          Class = [[TalkTo]],  
          Active = 0,  
          Base = [[palette.entities.botobjects.bot_chat]],  
          ContextualText = [[Yes lets attack!]],  
          InheritPos = 1,  
          MissionGiver = r2.RefId([[Client1_2668]]),  
          MissionSucceedText = [[CHARGE!!]],  
          MissionTarget = r2.RefId([[Client1_2771]]),  
          MissionText = [[Talk to Anisti if you wish to confirm the attack, or talk to Abyrixius or Gale to change your mind on the plan]],  
          Name = [[Mission: Talk to Ani Split]],  
          Repeatable = 0,  
          WaitValidationText = [[Are you ready to divide and conquer?]],  
          _Seed = 1154361419,  
          Behavior = {
            InstanceId = [[Client1_4313]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_4322]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_4325]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_4315]]),  
                    Action = {
                      InstanceId = [[Client1_4324]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4327]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_4318]]),  
                    Action = {
                      InstanceId = [[Client1_4326]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4329]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2683]]),  
                    Action = {
                      InstanceId = [[Client1_4328]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3238]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4331]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2746]]),  
                    Action = {
                      InstanceId = [[Client1_4330]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3243]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4333]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2660]]),  
                    Action = {
                      InstanceId = [[Client1_4332]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3230]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4335]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2664]]),  
                    Action = {
                      InstanceId = [[Client1_4334]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3236]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4337]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2725]]),  
                    Action = {
                      InstanceId = [[Client1_4336]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3881]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4339]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2704]]),  
                    Action = {
                      InstanceId = [[Client1_4338]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3877]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4341]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2668]]),  
                    Action = {
                      InstanceId = [[Client1_4340]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3870]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4343]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2771]]),  
                    Action = {
                      InstanceId = [[Client1_4342]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3868]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_4323]],  
                  Class = [[EventType]],  
                  Type = [[succeeded]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
          },  
          Position = {
            InstanceId = [[Client1_4314]],  
            Class = [[Position]],  
            x = 36279.21875,  
            y = -1107.21875,  
            z = -15
          }
        },  
        {
          InstanceId = [[Client1_4315]],  
          Class = [[TalkTo]],  
          Active = 0,  
          Base = [[palette.entities.botobjects.bot_chat]],  
          ContextualText = [[Change to <mission_target>'s plan]],  
          InheritPos = 1,  
          MissionGiver = r2.RefId([[Client1_2668]]),  
          MissionSucceedText = [[Ah glad you are switching, you had me worried there for a minute, lets go!]],  
          MissionTarget = r2.RefId([[Client1_2664]]),  
          MissionText = [[]],  
          Name = [[Mission: Talk to Gale Split]],  
          Repeatable = 0,  
          WaitValidationText = [[Hi there!  Attacking from the South would be the best idea in my opinion.  We will have more room to manuver there, and an attack in force will give us the upper hand!]],  
          _Seed = 1154361601,  
          Behavior = {
            InstanceId = [[Client1_4316]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_4345]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_4348]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_4318]]),  
                    Action = {
                      InstanceId = [[Client1_4347]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4350]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_4312]]),  
                    Action = {
                      InstanceId = [[Client1_4349]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4352]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2664]]),  
                    Action = {
                      InstanceId = [[Client1_4351]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3552]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4354]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2660]]),  
                    Action = {
                      InstanceId = [[Client1_4353]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3550]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4356]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2746]]),  
                    Action = {
                      InstanceId = [[Client1_4355]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3559]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4358]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2683]]),  
                    Action = {
                      InstanceId = [[Client1_4357]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_3240]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_4346]],  
                  Class = [[EventType]],  
                  Type = [[succeeded]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
          },  
          Position = {
            InstanceId = [[Client1_4317]],  
            Class = [[Position]],  
            x = 36277.71875,  
            y = -1108.46875,  
            z = -15
          }
        },  
        {
          InstanceId = [[Client1_4318]],  
          Class = [[TalkTo]],  
          Active = 0,  
          Base = [[palette.entities.botobjects.bot_chat]],  
          ContextualText = [[Change to <mission_target>'s plan]],  
          InheritPos = 1,  
          MissionGiver = r2.RefId([[Client1_2668]]),  
          MissionSucceedText = [[I am glad you had the courage to change your mind.  Lets move out!]],  
          MissionTarget = r2.RefId([[Client1_2660]]),  
          MissionText = [[]],  
          Name = [[Mission: Talk to Aby split]],  
          Repeatable = 0,  
          WaitValidationText = [[Sir, I think we should attack with all our units from the North.  They shall be overwhelmed by our mighty force before they even know what is going on!]],  
          _Seed = 1154361769,  
          Behavior = {
            InstanceId = [[Client1_4319]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_4360]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_4363]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_4312]]),  
                    Action = {
                      InstanceId = [[Client1_4362]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4365]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_4315]]),  
                    Action = {
                      InstanceId = [[Client1_4364]],  
                      Class = [[ActionType]],  
                      Type = [[deactivate]],  
                      Value = r2.RefId([[]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4367]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2668]]),  
                    Action = {
                      InstanceId = [[Client1_4366]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4165]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4369]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2771]]),  
                    Action = {
                      InstanceId = [[Client1_4368]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4169]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4371]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2725]]),  
                    Action = {
                      InstanceId = [[Client1_4370]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4159]])
                    }
                  },  
                  {
                    InstanceId = [[Client1_4373]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_2704]]),  
                    Action = {
                      InstanceId = [[Client1_4372]],  
                      Class = [[ActionType]],  
                      Type = [[begin activity sequence]],  
                      Value = r2.RefId([[Client1_4161]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_4361]],  
                  Class = [[EventType]],  
                  Type = [[succeeded]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
          },  
          Position = {
            InstanceId = [[Client1_4320]],  
            Class = [[Position]],  
            x = 36280.35938,  
            y = -1105.9375,  
            z = -15.890625
          }
        },  
        {
          InstanceId = [[Client1_4541]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Group 20]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_4539]],  
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
              InstanceId = [[Client1_4509]],  
              Class = [[NpcCustom]],  
              Angle = 0.09375,  
              ArmColor = 5,  
              ArmModel = 6701614,  
              AutoSpawn = 1,  
              Base = [[palette.entities.npcs.bandits.f_melee_tank_220]],  
              EyesColor = 6,  
              FeetColor = 1,  
              FeetModel = 6699310,  
              GabaritArmsWidth = 11,  
              GabaritBreastSize = 14,  
              GabaritHeight = 5,  
              GabaritLegsWidth = 7,  
              GabaritTorsoWidth = 1,  
              HairColor = 3,  
              HairType = 6700590,  
              HandsColor = 5,  
              HandsModel = 6700078,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6702126,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 7,  
              MorphTarget2 = 6,  
              MorphTarget3 = 3,  
              MorphTarget4 = 3,  
              MorphTarget5 = 5,  
              MorphTarget6 = 2,  
              MorphTarget7 = 3,  
              MorphTarget8 = 3,  
              Name = [[Meps]],  
              NoRespawn = 1,  
              Sex = 1,  
              Sheet = [[ring_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 1,  
              Tattoo = 2,  
              TrouserColor = 2,  
              TrouserModel = 6701102,  
              WeaponLeftHand = 6773038,  
              WeaponRightHand = 6754094,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4507]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_4553]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_4555]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Wander]],  
                        ActivityZoneId = r2.RefId([[Client1_2871]]),  
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
                InstanceId = [[Client1_4510]],  
                Class = [[Position]],  
                x = 36443.71875,  
                y = -1215.984375,  
                z = -19.484375
              }
            },  
            {
              InstanceId = [[Client1_4537]],  
              Class = [[NpcCustom]],  
              Angle = 0.09375,  
              ArmColor = 4,  
              ArmModel = 6701614,  
              AutoSpawn = 1,  
              Base = [[palette.entities.npcs.bandits.f_melee_tank_220]],  
              EyesColor = 6,  
              FeetColor = 5,  
              FeetModel = 6699310,  
              GabaritArmsWidth = 11,  
              GabaritBreastSize = 7,  
              GabaritHeight = 6,  
              GabaritLegsWidth = 5,  
              GabaritTorsoWidth = 0,  
              HairColor = 0,  
              HairType = 6700334,  
              HandsColor = 0,  
              HandsModel = 6700078,  
              InheritPos = 1,  
              JacketColor = 4,  
              JacketModel = 6702126,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 0,  
              MorphTarget2 = 7,  
              MorphTarget3 = 2,  
              MorphTarget4 = 2,  
              MorphTarget5 = 5,  
              MorphTarget6 = 5,  
              MorphTarget7 = 6,  
              MorphTarget8 = 2,  
              Name = [[Ibillo]],  
              NoRespawn = 1,  
              Sex = 0,  
              Sheet = [[ring_melee_tank_blunt_f4.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 1,  
              Tattoo = 23,  
              TrouserColor = 2,  
              TrouserModel = 6701102,  
              WeaponLeftHand = 6773294,  
              WeaponRightHand = 6752302,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4535]],  
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
                InstanceId = [[Client1_4538]],  
                Class = [[Position]],  
                x = 36444.35938,  
                y = -1214.875,  
                z = -19.5
              }
            },  
            {
              InstanceId = [[Client1_4489]],  
              Class = [[NpcCustom]],  
              Angle = 0.09375,  
              ArmColor = 4,  
              ArmModel = 6701614,  
              AutoSpawn = 1,  
              Base = [[palette.entities.npcs.bandits.f_melee_tank_220]],  
              EyesColor = 4,  
              FeetColor = 4,  
              FeetModel = 6699310,  
              GabaritArmsWidth = 8,  
              GabaritBreastSize = 1,  
              GabaritHeight = 10,  
              GabaritLegsWidth = 11,  
              GabaritTorsoWidth = 6,  
              HairColor = 3,  
              HairType = 6700334,  
              HandsColor = 4,  
              HandsModel = 6699822,  
              InheritPos = 1,  
              JacketColor = 5,  
              JacketModel = 6702126,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 0,  
              MorphTarget2 = 7,  
              MorphTarget3 = 2,  
              MorphTarget4 = 0,  
              MorphTarget5 = 3,  
              MorphTarget6 = 0,  
              MorphTarget7 = 2,  
              MorphTarget8 = 3,  
              Name = [[Thean]],  
              NoRespawn = 1,  
              Sex = 0,  
              Sheet = [[ring_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 1,  
              Tattoo = 20,  
              TrouserColor = 5,  
              TrouserModel = 6701102,  
              WeaponLeftHand = 6773038,  
              WeaponRightHand = 6754606,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4487]],  
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
                InstanceId = [[Client1_4490]],  
                Class = [[Position]],  
                x = 36443.98438,  
                y = -1216.859375,  
                z = -19.53125
              }
            },  
            {
              InstanceId = [[Client1_4455]],  
              Class = [[NpcCustom]],  
              Angle = 0.09375,  
              ArmColor = 3,  
              ArmModel = 6701614,  
              AutoSpawn = 1,  
              Base = [[palette.entities.npcs.bandits.f_melee_tank_220]],  
              EyesColor = 3,  
              FeetColor = 5,  
              FeetModel = 6699566,  
              GabaritArmsWidth = 5,  
              GabaritBreastSize = 10,  
              GabaritHeight = 1,  
              GabaritLegsWidth = 10,  
              GabaritTorsoWidth = 6,  
              HairColor = 5,  
              HairType = 6700590,  
              HandsColor = 1,  
              HandsModel = 6700078,  
              InheritPos = 1,  
              JacketColor = 1,  
              JacketModel = 6701870,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 3,  
              MorphTarget2 = 3,  
              MorphTarget3 = 1,  
              MorphTarget4 = 2,  
              MorphTarget5 = 6,  
              MorphTarget6 = 2,  
              MorphTarget7 = 3,  
              MorphTarget8 = 4,  
              Name = [[Xallo]],  
              NoRespawn = 1,  
              Sex = 1,  
              Sheet = [[ring_melee_tank_blunt_f4.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 1,  
              Tattoo = 23,  
              TrouserColor = 3,  
              TrouserModel = 6700846,  
              WeaponLeftHand = 6773294,  
              WeaponRightHand = 6752302,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4453]],  
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
                InstanceId = [[Client1_4456]],  
                Class = [[Position]],  
                x = 36443.01563,  
                y = -1217.75,  
                z = -19.46875
              }
            },  
            {
              InstanceId = [[Client1_4493]],  
              Class = [[NpcCustom]],  
              Angle = 0.09375,  
              ArmColor = 4,  
              ArmModel = 6701614,  
              AutoSpawn = 1,  
              Base = [[palette.entities.npcs.bandits.f_melee_tank_220]],  
              EyesColor = 5,  
              FeetColor = 1,  
              FeetModel = 6699566,  
              GabaritArmsWidth = 5,  
              GabaritBreastSize = 11,  
              GabaritHeight = 6,  
              GabaritLegsWidth = 5,  
              GabaritTorsoWidth = 1,  
              HairColor = 3,  
              HairType = 6700590,  
              HandsColor = 4,  
              HandsModel = 6699822,  
              InheritPos = 1,  
              JacketColor = 4,  
              JacketModel = 6701870,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 6,  
              MorphTarget2 = 7,  
              MorphTarget3 = 2,  
              MorphTarget4 = 2,  
              MorphTarget5 = 4,  
              MorphTarget6 = 4,  
              MorphTarget7 = 5,  
              MorphTarget8 = 4,  
              Name = [[Xakos]],  
              NoRespawn = 1,  
              Sex = 0,  
              Sheet = [[ring_melee_tank_pierce_f4.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 1,  
              Tattoo = 26,  
              TrouserColor = 1,  
              TrouserModel = 6701102,  
              WeaponLeftHand = 6773038,  
              WeaponRightHand = 6753582,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4491]],  
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
                InstanceId = [[Client1_4494]],  
                Class = [[Position]],  
                x = 36441.95313,  
                y = -1216.734375,  
                z = -19.3125
              }
            },  
            {
              InstanceId = [[Client1_4485]],  
              Class = [[NpcCustom]],  
              Angle = 0.09375,  
              ArmColor = 0,  
              ArmModel = 6701614,  
              AutoSpawn = 1,  
              Base = [[palette.entities.npcs.bandits.f_melee_tank_220]],  
              EyesColor = 6,  
              FeetColor = 5,  
              FeetModel = 6699310,  
              GabaritArmsWidth = 2,  
              GabaritBreastSize = 4,  
              GabaritHeight = 6,  
              GabaritLegsWidth = 10,  
              GabaritTorsoWidth = 5,  
              HairColor = 3,  
              HairType = 6700334,  
              HandsColor = 3,  
              HandsModel = 6699822,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6702126,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 3,  
              MorphTarget2 = 0,  
              MorphTarget3 = 5,  
              MorphTarget4 = 5,  
              MorphTarget5 = 4,  
              MorphTarget6 = 3,  
              MorphTarget7 = 1,  
              MorphTarget8 = 1,  
              Name = [[Kyron]],  
              NoRespawn = 1,  
              Sex = 1,  
              Sheet = [[ring_melee_tank_pierce_f4.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 1,  
              Tattoo = 18,  
              TrouserColor = 1,  
              TrouserModel = 6700846,  
              WeaponLeftHand = 6773038,  
              WeaponRightHand = 6753582,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4483]],  
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
                InstanceId = [[Client1_4486]],  
                Class = [[Position]],  
                x = 36444.0625,  
                y = -1218.421875,  
                z = -19.625
              }
            },  
            {
              InstanceId = [[Client1_4525]],  
              Class = [[NpcCustom]],  
              Angle = 0.09375,  
              ArmColor = 1,  
              ArmModel = 6701614,  
              AutoSpawn = 1,  
              Base = [[palette.entities.npcs.bandits.f_melee_tank_220]],  
              EyesColor = 6,  
              FeetColor = 0,  
              FeetModel = 6699566,  
              GabaritArmsWidth = 11,  
              GabaritBreastSize = 6,  
              GabaritHeight = 0,  
              GabaritLegsWidth = 3,  
              GabaritTorsoWidth = 4,  
              HairColor = 5,  
              HairType = 6700590,  
              HandsColor = 1,  
              HandsModel = 6700078,  
              InheritPos = 1,  
              JacketColor = 1,  
              JacketModel = 6701870,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 6,  
              MorphTarget2 = 0,  
              MorphTarget3 = 6,  
              MorphTarget4 = 4,  
              MorphTarget5 = 4,  
              MorphTarget6 = 0,  
              MorphTarget7 = 6,  
              MorphTarget8 = 6,  
              Name = [[Kyron]],  
              NoRespawn = 1,  
              Sex = 0,  
              Sheet = [[ring_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 1,  
              Tattoo = 26,  
              TrouserColor = 4,  
              TrouserModel = 6701102,  
              WeaponLeftHand = 6773038,  
              WeaponRightHand = 6754606,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4523]],  
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
                InstanceId = [[Client1_4526]],  
                Class = [[Position]],  
                x = 36439.875,  
                y = -1216.8125,  
                z = -18.96875
              }
            },  
            {
              InstanceId = [[Client1_4501]],  
              Class = [[NpcCustom]],  
              Angle = 0.09375,  
              ArmColor = 3,  
              ArmModel = 6701614,  
              AutoSpawn = 1,  
              Base = [[palette.entities.npcs.bandits.f_melee_tank_220]],  
              EyesColor = 7,  
              FeetColor = 0,  
              FeetModel = 6699566,  
              GabaritArmsWidth = 0,  
              GabaritBreastSize = 13,  
              GabaritHeight = 0,  
              GabaritLegsWidth = 6,  
              GabaritTorsoWidth = 2,  
              HairColor = 1,  
              HairType = 6700590,  
              HandsColor = 4,  
              HandsModel = 6700078,  
              InheritPos = 1,  
              JacketColor = 1,  
              JacketModel = 6701870,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 2,  
              MorphTarget2 = 3,  
              MorphTarget3 = 1,  
              MorphTarget4 = 5,  
              MorphTarget5 = 7,  
              MorphTarget6 = 3,  
              MorphTarget7 = 1,  
              MorphTarget8 = 2,  
              Name = [[Dyrius]],  
              NoRespawn = 1,  
              Sex = 0,  
              Sheet = [[ring_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 1,  
              Tattoo = 11,  
              TrouserColor = 5,  
              TrouserModel = 6700846,  
              WeaponLeftHand = 6773294,  
              WeaponRightHand = 6754862,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4499]],  
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
                InstanceId = [[Client1_4502]],  
                Class = [[Position]],  
                x = 36439.53125,  
                y = -1217,  
                z = -18.90625
              }
            },  
            {
              InstanceId = [[Client1_4529]],  
              Class = [[NpcCustom]],  
              Angle = 0.09375,  
              ArmColor = 5,  
              ArmModel = 6701614,  
              AutoSpawn = 1,  
              Base = [[palette.entities.npcs.bandits.f_melee_tank_220]],  
              EyesColor = 0,  
              FeetColor = 1,  
              FeetModel = 6699310,  
              GabaritArmsWidth = 4,  
              GabaritBreastSize = 11,  
              GabaritHeight = 1,  
              GabaritLegsWidth = 13,  
              GabaritTorsoWidth = 1,  
              HairColor = 1,  
              HairType = 6700334,  
              HandsColor = 3,  
              HandsModel = 6700078,  
              InheritPos = 1,  
              JacketColor = 5,  
              JacketModel = 6702126,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 2,  
              MorphTarget2 = 6,  
              MorphTarget3 = 2,  
              MorphTarget4 = 4,  
              MorphTarget5 = 1,  
              MorphTarget6 = 5,  
              MorphTarget7 = 3,  
              MorphTarget8 = 3,  
              Name = [[Xathus]],  
              NoRespawn = 1,  
              Sex = 1,  
              Sheet = [[ring_melee_tank_blunt_f4.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 1,  
              Tattoo = 20,  
              TrouserColor = 5,  
              TrouserModel = 6700846,  
              WeaponLeftHand = 6773038,  
              WeaponRightHand = 6752046,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4527]],  
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
                InstanceId = [[Client1_4530]],  
                Class = [[Position]],  
                x = 36438.01563,  
                y = -1215.890625,  
                z = -18.625
              }
            },  
            {
              InstanceId = [[Client1_4533]],  
              Class = [[NpcCustom]],  
              Angle = 0.09375,  
              ArmColor = 4,  
              ArmModel = 6701614,  
              AutoSpawn = 1,  
              Base = [[palette.entities.npcs.bandits.f_melee_tank_220]],  
              EyesColor = 6,  
              FeetColor = 1,  
              FeetModel = 6699310,  
              GabaritArmsWidth = 11,  
              GabaritBreastSize = 10,  
              GabaritHeight = 3,  
              GabaritLegsWidth = 2,  
              GabaritTorsoWidth = 4,  
              HairColor = 4,  
              HairType = 6700590,  
              HandsColor = 4,  
              HandsModel = 6699822,  
              InheritPos = 1,  
              JacketColor = 5,  
              JacketModel = 6701870,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 2,  
              MorphTarget2 = 4,  
              MorphTarget3 = 4,  
              MorphTarget4 = 6,  
              MorphTarget5 = 3,  
              MorphTarget6 = 2,  
              MorphTarget7 = 5,  
              MorphTarget8 = 6,  
              Name = [[Ulyion]],  
              NoRespawn = 1,  
              Sex = 1,  
              Sheet = [[ring_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 1,  
              Tattoo = 1,  
              TrouserColor = 0,  
              TrouserModel = 6700846,  
              WeaponLeftHand = 6773038,  
              WeaponRightHand = 6754094,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4531]],  
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
                InstanceId = [[Client1_4534]],  
                Class = [[Position]],  
                x = 36438.46875,  
                y = -1218.078125,  
                z = -18.78125
              }
            },  
            {
              InstanceId = [[Client1_4470]],  
              Class = [[NpcCustom]],  
              Angle = 0.09375,  
              ArmColor = 2,  
              ArmModel = 6701358,  
              AutoSpawn = 1,  
              Base = [[palette.entities.npcs.bandits.f_melee_tank_220]],  
              EyesColor = 5,  
              FeetColor = 3,  
              FeetModel = 6699310,  
              GabaritArmsWidth = 1,  
              GabaritBreastSize = 5,  
              GabaritHeight = 7,  
              GabaritLegsWidth = 4,  
              GabaritTorsoWidth = 2,  
              HairColor = 5,  
              HairType = 6700334,  
              HandsColor = 3,  
              HandsModel = 6699822,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6701870,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 6,  
              MorphTarget2 = 4,  
              MorphTarget3 = 7,  
              MorphTarget4 = 7,  
              MorphTarget5 = 3,  
              MorphTarget6 = 7,  
              MorphTarget7 = 7,  
              MorphTarget8 = 1,  
              Name = [[Iodix]],  
              NoRespawn = 1,  
              Sex = 1,  
              Sheet = [[ring_melee_tank_pierce_f4.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 1,  
              Tattoo = 18,  
              TrouserColor = 5,  
              TrouserModel = 6701102,  
              WeaponLeftHand = 6773294,  
              WeaponRightHand = 6753838,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4468]],  
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
                InstanceId = [[Client1_4471]],  
                Class = [[Position]],  
                x = 36439.59375,  
                y = -1218.921875,  
                z = -19.03125
              }
            },  
            {
              InstanceId = [[Client1_4521]],  
              Class = [[NpcCustom]],  
              Angle = 0.09375,  
              ArmColor = 4,  
              ArmModel = 6701358,  
              AutoSpawn = 1,  
              Base = [[palette.entities.npcs.bandits.f_melee_tank_220]],  
              EyesColor = 2,  
              FeetColor = 2,  
              FeetModel = 6699310,  
              GabaritArmsWidth = 13,  
              GabaritBreastSize = 8,  
              GabaritHeight = 12,  
              GabaritLegsWidth = 7,  
              GabaritTorsoWidth = 4,  
              HairColor = 2,  
              HairType = 6700590,  
              HandsColor = 3,  
              HandsModel = 6700078,  
              InheritPos = 1,  
              JacketColor = 4,  
              JacketModel = 6701870,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 1,  
              MorphTarget2 = 0,  
              MorphTarget3 = 6,  
              MorphTarget4 = 7,  
              MorphTarget5 = 0,  
              MorphTarget6 = 4,  
              MorphTarget7 = 0,  
              MorphTarget8 = 3,  
              Name = [[Detheus]],  
              NoRespawn = 1,  
              Sex = 1,  
              Sheet = [[ring_melee_tank_blunt_f4.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 1,  
              Tattoo = 21,  
              TrouserColor = 3,  
              TrouserModel = 6700846,  
              WeaponLeftHand = 6773294,  
              WeaponRightHand = 6752302,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4519]],  
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
                InstanceId = [[Client1_4522]],  
                Class = [[Position]],  
                x = 36439.04688,  
                y = -1220.265625,  
                z = -19.078125
              }
            },  
            {
              InstanceId = [[Client1_4497]],  
              Class = [[NpcCustom]],  
              Angle = 0.09375,  
              ArmColor = 3,  
              ArmModel = 6701358,  
              AutoSpawn = 1,  
              Base = [[palette.entities.npcs.bandits.f_melee_tank_220]],  
              EyesColor = 4,  
              FeetColor = 3,  
              FeetModel = 6699310,  
              GabaritArmsWidth = 10,  
              GabaritBreastSize = 1,  
              GabaritHeight = 11,  
              GabaritLegsWidth = 13,  
              GabaritTorsoWidth = 6,  
              HairColor = 0,  
              HairType = 6700590,  
              HandsColor = 3,  
              HandsModel = 6699822,  
              InheritPos = 1,  
              JacketColor = 2,  
              JacketModel = 6701870,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 7,  
              MorphTarget2 = 6,  
              MorphTarget3 = 3,  
              MorphTarget4 = 7,  
              MorphTarget5 = 3,  
              MorphTarget6 = 7,  
              MorphTarget7 = 2,  
              MorphTarget8 = 4,  
              Name = [[Xymus]],  
              NoRespawn = 1,  
              Sex = 0,  
              Sheet = [[ring_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 1,  
              Tattoo = 26,  
              TrouserColor = 4,  
              TrouserModel = 6700846,  
              WeaponLeftHand = 6773038,  
              WeaponRightHand = 6754094,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4495]],  
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
                InstanceId = [[Client1_4498]],  
                Class = [[Position]],  
                x = 36441.29688,  
                y = -1217.84375,  
                z = -19.234375
              }
            },  
            {
              InstanceId = [[Client1_4513]],  
              Class = [[NpcCustom]],  
              Angle = 0.09375,  
              ArmColor = 5,  
              ArmModel = 6701614,  
              AutoSpawn = 1,  
              Base = [[palette.entities.npcs.bandits.f_melee_tank_220]],  
              EyesColor = 2,  
              FeetColor = 1,  
              FeetModel = 6699566,  
              GabaritArmsWidth = 0,  
              GabaritBreastSize = 14,  
              GabaritHeight = 2,  
              GabaritLegsWidth = 9,  
              GabaritTorsoWidth = 1,  
              HairColor = 3,  
              HairType = 6700334,  
              HandsColor = 4,  
              HandsModel = 6699822,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6701870,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 3,  
              MorphTarget2 = 5,  
              MorphTarget3 = 1,  
              MorphTarget4 = 4,  
              MorphTarget5 = 2,  
              MorphTarget6 = 4,  
              MorphTarget7 = 4,  
              MorphTarget8 = 7,  
              Name = [[Boean]],  
              NoRespawn = 1,  
              Sex = 1,  
              Sheet = [[ring_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 1,  
              Tattoo = 16,  
              TrouserColor = 3,  
              TrouserModel = 6701102,  
              WeaponLeftHand = 6773294,  
              WeaponRightHand = 6754862,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4511]],  
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
                InstanceId = [[Client1_4514]],  
                Class = [[Position]],  
                x = 36442.32813,  
                y = -1218.6875,  
                z = -19.484375
              }
            },  
            {
              InstanceId = [[Client1_4474]],  
              Class = [[NpcCustom]],  
              Angle = 0.09375,  
              ArmColor = 3,  
              ArmModel = 6701614,  
              AutoSpawn = 1,  
              Base = [[palette.entities.npcs.bandits.f_melee_tank_220]],  
              EyesColor = 2,  
              FeetColor = 0,  
              FeetModel = 6699310,  
              GabaritArmsWidth = 0,  
              GabaritBreastSize = 7,  
              GabaritHeight = 14,  
              GabaritLegsWidth = 1,  
              GabaritTorsoWidth = 7,  
              HairColor = 0,  
              HairType = 6700590,  
              HandsColor = 3,  
              HandsModel = 6699822,  
              InheritPos = 1,  
              JacketColor = 1,  
              JacketModel = 6701870,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 5,  
              MorphTarget2 = 6,  
              MorphTarget3 = 3,  
              MorphTarget4 = 0,  
              MorphTarget5 = 0,  
              MorphTarget6 = 6,  
              MorphTarget7 = 2,  
              MorphTarget8 = 6,  
              Name = [[Eulion]],  
              NoRespawn = 1,  
              Sex = 0,  
              Sheet = [[ring_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 1,  
              Tattoo = 21,  
              TrouserColor = 3,  
              TrouserModel = 6700846,  
              WeaponLeftHand = 6773038,  
              WeaponRightHand = 6754094,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4472]],  
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
                InstanceId = [[Client1_4475]],  
                Class = [[Position]],  
                x = 36441.21875,  
                y = -1219.09375,  
                z = -19.328125
              }
            },  
            {
              InstanceId = [[Client1_4517]],  
              Class = [[NpcCustom]],  
              Angle = 0.09375,  
              ArmColor = 4,  
              ArmModel = 6701358,  
              AutoSpawn = 1,  
              Base = [[palette.entities.npcs.bandits.f_melee_tank_220]],  
              EyesColor = 2,  
              FeetColor = 2,  
              FeetModel = 6699566,  
              GabaritArmsWidth = 6,  
              GabaritBreastSize = 8,  
              GabaritHeight = 11,  
              GabaritLegsWidth = 14,  
              GabaritTorsoWidth = 7,  
              HairColor = 4,  
              HairType = 6700334,  
              HandsColor = 5,  
              HandsModel = 6699822,  
              InheritPos = 1,  
              JacketColor = 1,  
              JacketModel = 6701870,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 5,  
              MorphTarget2 = 7,  
              MorphTarget3 = 5,  
              MorphTarget4 = 0,  
              MorphTarget5 = 5,  
              MorphTarget6 = 1,  
              MorphTarget7 = 5,  
              MorphTarget8 = 4,  
              Name = [[Zeps]],  
              NoRespawn = 1,  
              Sex = 1,  
              Sheet = [[ring_melee_tank_pierce_f4.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 1,  
              Tattoo = 31,  
              TrouserColor = 3,  
              TrouserModel = 6701102,  
              WeaponLeftHand = 6773294,  
              WeaponRightHand = 6753838,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4515]],  
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
                InstanceId = [[Client1_4518]],  
                Class = [[Position]],  
                x = 36440.89063,  
                y = -1220.21875,  
                z = -19.375
              }
            },  
            {
              InstanceId = [[Client1_4466]],  
              Class = [[NpcCustom]],  
              Angle = 0.09375,  
              ArmColor = 4,  
              ArmModel = 6701614,  
              Base = [[palette.entities.npcs.bandits.f_melee_tank_220]],  
              EyesColor = 2,  
              FeetColor = 4,  
              FeetModel = 6699566,  
              GabaritArmsWidth = 11,  
              GabaritBreastSize = 3,  
              GabaritHeight = 1,  
              GabaritLegsWidth = 13,  
              GabaritTorsoWidth = 5,  
              HairColor = 0,  
              HairType = 6700590,  
              HandsColor = 3,  
              HandsModel = 6699822,  
              InheritPos = 1,  
              JacketColor = 2,  
              JacketModel = 6702126,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 0,  
              MorphTarget2 = 2,  
              MorphTarget3 = 7,  
              MorphTarget4 = 5,  
              MorphTarget5 = 4,  
              MorphTarget6 = 3,  
              MorphTarget7 = 4,  
              MorphTarget8 = 5,  
              Name = [[Dynix]],  
              NoRespawn = 1,  
              Sex = 0,  
              Sheet = [[ring_melee_tank_blunt_f4.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 1,  
              Tattoo = 7,  
              TrouserColor = 0,  
              TrouserModel = 6700846,  
              WeaponLeftHand = 6773038,  
              WeaponRightHand = 6752046,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4464]],  
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
                InstanceId = [[Client1_4467]],  
                Class = [[Position]],  
                x = 36439.96875,  
                y = -1221.109375,  
                z = -19.359375
              }
            },  
            {
              InstanceId = [[Client1_4505]],  
              Class = [[NpcCustom]],  
              Angle = 0.09375,  
              ArmColor = 0,  
              ArmModel = 6701358,  
              AutoSpawn = 1,  
              Base = [[palette.entities.npcs.bandits.f_melee_tank_220]],  
              EyesColor = 1,  
              FeetColor = 1,  
              FeetModel = 6699310,  
              GabaritArmsWidth = 2,  
              GabaritBreastSize = 9,  
              GabaritHeight = 12,  
              GabaritLegsWidth = 14,  
              GabaritTorsoWidth = 1,  
              HairColor = 1,  
              HairType = 6700590,  
              HandsColor = 5,  
              HandsModel = 6699822,  
              InheritPos = 1,  
              JacketColor = 2,  
              JacketModel = 6702126,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 7,  
              MorphTarget2 = 0,  
              MorphTarget3 = 2,  
              MorphTarget4 = 7,  
              MorphTarget5 = 6,  
              MorphTarget6 = 2,  
              MorphTarget7 = 1,  
              MorphTarget8 = 2,  
              Name = [[Lyton]],  
              NoRespawn = 1,  
              Sex = 1,  
              Sheet = [[ring_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 1,  
              Tattoo = 18,  
              TrouserColor = 2,  
              TrouserModel = 6700846,  
              WeaponLeftHand = 6773038,  
              WeaponRightHand = 6754094,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4503]],  
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
                InstanceId = [[Client1_4506]],  
                Class = [[Position]],  
                x = 36441.0625,  
                y = -1221.171875,  
                z = -19.5625
              }
            },  
            {
              InstanceId = [[Client1_4459]],  
              Class = [[NpcCustom]],  
              Angle = 0.09375,  
              ArmColor = 5,  
              ArmModel = 6701358,  
              AutoSpawn = 1,  
              Base = [[palette.entities.npcs.bandits.f_melee_tank_220]],  
              EyesColor = 6,  
              FeetColor = 1,  
              FeetModel = 6699566,  
              GabaritArmsWidth = 14,  
              GabaritBreastSize = 2,  
              GabaritHeight = 1,  
              GabaritLegsWidth = 3,  
              GabaritTorsoWidth = 1,  
              HairColor = 3,  
              HairType = 6700590,  
              HandsColor = 5,  
              HandsModel = 6700078,  
              InheritPos = 1,  
              JacketColor = 3,  
              JacketModel = 6702126,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 1,  
              MorphTarget2 = 1,  
              MorphTarget3 = 7,  
              MorphTarget4 = 3,  
              MorphTarget5 = 2,  
              MorphTarget6 = 1,  
              MorphTarget7 = 1,  
              MorphTarget8 = 0,  
              Name = [[Boedon]],  
              NoRespawn = 1,  
              Sex = 1,  
              Sheet = [[ring_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_fyros_female.creature]],  
              Speed = 1,  
              Tattoo = 29,  
              TrouserColor = 5,  
              TrouserModel = 6700846,  
              WeaponLeftHand = 6773038,  
              WeaponRightHand = 6754094,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4457]],  
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
                InstanceId = [[Client1_4460]],  
                Class = [[Position]],  
                x = 36442.75,  
                y = -1220.109375,  
                z = -19.640625
              }
            },  
            {
              InstanceId = [[Client1_4481]],  
              Class = [[NpcCustom]],  
              Angle = 0.09375,  
              ArmColor = 2,  
              ArmModel = 6701358,  
              AutoSpawn = 1,  
              Base = [[palette.entities.npcs.bandits.f_melee_tank_220]],  
              EyesColor = 2,  
              FeetColor = 2,  
              FeetModel = 6699566,  
              GabaritArmsWidth = 9,  
              GabaritBreastSize = 13,  
              GabaritHeight = 11,  
              GabaritLegsWidth = 6,  
              GabaritTorsoWidth = 3,  
              HairColor = 0,  
              HairType = 6700590,  
              HandsColor = 5,  
              HandsModel = 6700078,  
              InheritPos = 1,  
              JacketColor = 2,  
              JacketModel = 6701870,  
              Level = 3,  
              LinkColor = 0,  
              MorphTarget1 = 7,  
              MorphTarget2 = 3,  
              MorphTarget3 = 1,  
              MorphTarget4 = 2,  
              MorphTarget5 = 0,  
              MorphTarget6 = 6,  
              MorphTarget7 = 2,  
              MorphTarget8 = 0,  
              Name = [[Iodon]],  
              NoRespawn = 1,  
              Sex = 0,  
              Sheet = [[ring_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 1,  
              Tattoo = 19,  
              TrouserColor = 1,  
              TrouserModel = 6700846,  
              WeaponLeftHand = 6773038,  
              WeaponRightHand = 6754606,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4479]],  
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
                InstanceId = [[Client1_4482]],  
                Class = [[Position]],  
                x = 36443.82813,  
                y = -1219.703125,  
                z = -19.75
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_4540]],  
            Class = [[Position]],  
            x = 0.125,  
            y = -0.0625,  
            z = -0.015625
          }
        },  
        {
          InstanceId = [[Client1_4775]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[In position]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_4773]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_4776]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_4777]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_4778]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_4774]],  
            Class = [[Position]],  
            x = 36261.09375,  
            y = -1074.046875,  
            z = -11
          },  
          SubComponents = {
          }
        }
      },  
      Position = {
        InstanceId = [[Client1_2632]],  
        Class = [[Position]],  
        x = 0,  
        y = 0,  
        z = 0
      }
    },  
    {
      InstanceId = [[Client1_4558]],  
      Class = [[Act]],  
      Version = 6,  
      InheritPos = 1,  
      LocationId = [[Client1_4560]],  
      ManualWeather = 1,  
      Name = [[Act 4]],  
      Season = 0,  
      ShortDescription = [[Safety has been found with the aid of the Kami.]],  
      Title = [[]],  
      WeatherValue = 444,  
      ActivitiesIds = {
      },  
      Behavior = {
        InstanceId = [[Client1_4556]],  
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
          InstanceId = [[Client1_4559]],  
          Class = [[DefaultFeature]],  
          Components = {
            {
              InstanceId = [[Client1_4571]],  
              Class = [[Npc]],  
              Angle = -2.71875,  
              Base = [[palette.entities.botobjects.banner_kami]],  
              InheritPos = 1,  
              Name = [[kami banner 1]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4569]],  
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
                InstanceId = [[Client1_4572]],  
                Class = [[Position]],  
                x = 32090.26563,  
                y = -1380.703125,  
                z = -27.34375
              }
            },  
            {
              InstanceId = [[Client1_4575]],  
              Class = [[Npc]],  
              Angle = -2.75,  
              Base = [[palette.entities.npcs.kami.kami_preacher_2_f]],  
              InheritPos = 1,  
              Name = [[Kami Representative]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4573]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_4763]],  
                    Class = [[LogicEntityAction]],  
                    Name = [[]],  
                    Actions = {
                      {
                        InstanceId = [[Client1_4766]],  
                        Class = [[ActionStep]],  
                        Entity = r2.RefId([[Client1_4742]]),  
                        Action = {
                          InstanceId = [[Client1_4765]],  
                          Class = [[ActionType]],  
                          Type = [[starts dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Conditions = {
                      {
                        InstanceId = [[Client1_5841]],  
                        Class = [[ConditionStep]],  
                        Entity = r2.RefId([[Client1_5818]]),  
                        Condition = {
                          InstanceId = [[Client1_5840]],  
                          Class = [[ConditionType]],  
                          Type = [[is inactive]],  
                          Value = r2.RefId([[]])
                        }
                      },  
                      {
                        InstanceId = [[Client1_5843]],  
                        Class = [[ConditionStep]],  
                        Entity = r2.RefId([[Client1_4742]]),  
                        Condition = {
                          InstanceId = [[Client1_5842]],  
                          Class = [[ConditionType]],  
                          Type = [[is not in dialog]],  
                          Value = r2.RefId([[]])
                        }
                      }
                    },  
                    Event = {
                      InstanceId = [[Client1_4764]],  
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
                InstanceId = [[Client1_4576]],  
                Class = [[Position]],  
                x = 32099.0625,  
                y = -1373.34375,  
                z = -27.6875
              }
            },  
            {
              InstanceId = [[Client1_4697]],  
              Class = [[NpcCustom]],  
              Angle = 2.890625,  
              ArmColor = 4,  
              ArmModel = 6714926,  
              Base = [[palette.entities.npcs.civils.m_civil_220]],  
              EyesColor = 5,  
              FeetColor = 4,  
              FeetModel = 6715950,  
              GabaritArmsWidth = 8,  
              GabaritBreastSize = 8,  
              GabaritHeight = 0,  
              GabaritLegsWidth = 8,  
              GabaritTorsoWidth = 3,  
              HairColor = 2,  
              HairType = 5622574,  
              HandsColor = 4,  
              HandsModel = 6713902,  
              InheritPos = 1,  
              JacketColor = 4,  
              JacketModel = 6715438,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 3,  
              MorphTarget2 = 5,  
              MorphTarget3 = 2,  
              MorphTarget4 = 5,  
              MorphTarget5 = 5,  
              MorphTarget6 = 6,  
              MorphTarget7 = 3,  
              MorphTarget8 = 7,  
              Name = [[Gine Nidera]],  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_civil_light_melee_blunt_f2.creature]],  
              SheetClient = [[basic_matis_female.creature]],  
              Speed = 0,  
              Tattoo = 16,  
              TrouserColor = 4,  
              TrouserModel = 6714414,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 0,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4695]],  
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
                InstanceId = [[Client1_4698]],  
                Class = [[Position]],  
                x = 32092.89063,  
                y = -1381.25,  
                z = -27.5625
              }
            },  
            {
              InstanceId = [[Client1_4693]],  
              Class = [[NpcCustom]],  
              Angle = 2.890625,  
              ArmColor = 1,  
              ArmModel = 6717230,  
              Base = [[palette.entities.npcs.civils.m_civil_220]],  
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
              InheritPos = 1,  
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
              Name = [[Trigio Nidera]],  
              Notes = [[]],  
              Sex = 1,  
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
                InstanceId = [[Client1_4691]],  
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
                InstanceId = [[Client1_4694]],  
                Class = [[Position]],  
                x = 32094.09375,  
                y = -1379.609375,  
                z = -27.671875
              }
            },  
            {
              InstanceId = [[Client1_4715]],  
              Class = [[NpcCustom]],  
              Angle = -1.953125,  
              ArmColor = 4,  
              ArmModel = 6722862,  
              Base = [[palette.entities.npcs.guards.t_guard_245]],  
              EyesColor = 3,  
              FeetColor = 4,  
              FeetModel = 6720814,  
              GabaritArmsWidth = 6,  
              GabaritBreastSize = 6,  
              GabaritHeight = 6,  
              GabaritLegsWidth = 8,  
              GabaritTorsoWidth = 4,  
              HairColor = 4,  
              HairType = 6721838,  
              HandsColor = 4,  
              HandsModel = 6721326,  
              InheritPos = 1,  
              JacketColor = 4,  
              JacketModel = 6723374,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 3,  
              MorphTarget2 = 2,  
              MorphTarget3 = 3,  
              MorphTarget4 = 2,  
              MorphTarget5 = 4,  
              MorphTarget6 = 3,  
              MorphTarget7 = 0,  
              MorphTarget8 = 3,  
              Name = [[Gale Brapie]],  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_guard_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_tryker_female.creature]],  
              Speed = 0,  
              Tattoo = 16,  
              TrouserColor = 4,  
              TrouserModel = 6722350,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6766894,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4713]],  
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
                InstanceId = [[Client1_4716]],  
                Class = [[Position]],  
                x = 32094.14063,  
                y = -1369.65625,  
                z = -27.21875
              }
            },  
            {
              InstanceId = [[Client1_4719]],  
              Class = [[NpcCustom]],  
              Angle = -1.953125,  
              ArmColor = 4,  
              ArmModel = 6701614,  
              Base = [[palette.entities.npcs.guards.f_guard_245]],  
              EyesColor = 0,  
              FeetColor = 4,  
              FeetModel = 6699566,  
              GabaritArmsWidth = 7,  
              GabaritBreastSize = 5,  
              GabaritHeight = 1,  
              GabaritLegsWidth = 8,  
              GabaritTorsoWidth = 2,  
              HairColor = 4,  
              HairType = 6700590,  
              HandsColor = 4,  
              HandsModel = 6700078,  
              InheritPos = 1,  
              JacketColor = 4,  
              JacketModel = 6702126,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 4,  
              MorphTarget2 = 3,  
              MorphTarget3 = 6,  
              MorphTarget4 = 6,  
              MorphTarget5 = 1,  
              MorphTarget6 = 6,  
              MorphTarget7 = 3,  
              MorphTarget8 = 2,  
              Name = [[Abyrixius Iodix]],  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_guard_melee_tank_blunt_f4.creature]],  
              SheetClient = [[basic_fyros_male.creature]],  
              Speed = 0,  
              Tattoo = 28,  
              TrouserColor = 4,  
              TrouserModel = 6701102,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6755118,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4717]],  
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
                InstanceId = [[Client1_4720]],  
                Class = [[Position]],  
                x = 32095.15625,  
                y = -1370.140625,  
                z = -27.328125
              }
            },  
            {
              InstanceId = [[Client1_4708]],  
              Class = [[NpcCustom]],  
              Angle = -1.953125,  
              ArmColor = 0,  
              ArmModel = 6733614,  
              Base = [[palette.entities.npcs.guards.z_guard_245]],  
              EyesColor = 2,  
              FeetColor = 0,  
              FeetModel = 6731566,  
              GabaritArmsWidth = 9,  
              GabaritBreastSize = 12,  
              GabaritHeight = 14,  
              GabaritLegsWidth = 6,  
              GabaritTorsoWidth = 7,  
              HairColor = 0,  
              HairType = 6732590,  
              HandsColor = 0,  
              HandsModel = 6732078,  
              InheritPos = 1,  
              JacketColor = 0,  
              JacketModel = 6734126,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 3,  
              MorphTarget2 = 1,  
              MorphTarget3 = 0,  
              MorphTarget4 = 1,  
              MorphTarget5 = 6,  
              MorphTarget6 = 5,  
              MorphTarget7 = 5,  
              MorphTarget8 = 1,  
              Name = [[Be-ci Kuani]],  
              Notes = [[]],  
              Sex = 1,  
              Sheet = [[ring_guard_melee_tank_slash_f4.creature]],  
              SheetClient = [[basic_zorai_male.creature]],  
              Speed = 0,  
              Tattoo = 3,  
              TrouserColor = 0,  
              TrouserModel = 6733102,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6771502,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4706]],  
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
                InstanceId = [[Client1_4709]],  
                Class = [[Position]],  
                x = 32096.57813,  
                y = -1371.65625,  
                z = -27.5
              }
            },  
            {
              InstanceId = [[Client1_4704]],  
              Class = [[NpcCustom]],  
              Angle = -1.953125,  
              ArmColor = 4,  
              ArmModel = 6712110,  
              Base = [[palette.entities.npcs.guards.m_guard_245]],  
              EyesColor = 3,  
              FeetColor = 4,  
              FeetModel = 6710062,  
              GabaritArmsWidth = 7,  
              GabaritBreastSize = 4,  
              GabaritHeight = 14,  
              GabaritLegsWidth = 8,  
              GabaritTorsoWidth = 5,  
              HairColor = 4,  
              HairType = 6711086,  
              HandsColor = 4,  
              HandsModel = 6710574,  
              InheritPos = 1,  
              JacketColor = 4,  
              JacketModel = 6712622,  
              Level = 0,  
              LinkColor = 0,  
              MorphTarget1 = 2,  
              MorphTarget2 = 2,  
              MorphTarget3 = 2,  
              MorphTarget4 = 4,  
              MorphTarget5 = 3,  
              MorphTarget6 = 3,  
              MorphTarget7 = 4,  
              MorphTarget8 = 3,  
              Name = [[Anisti Pillo]],  
              Notes = [[]],  
              Sex = 0,  
              Sheet = [[ring_guard_melee_tank_blunt_f4.creature]],  
              SheetClient = [[basic_matis_male.creature]],  
              Speed = 0,  
              Tattoo = 14,  
              TrouserColor = 4,  
              TrouserModel = 6711598,  
              WeaponLeftHand = 0,  
              WeaponRightHand = 6760238,  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4702]],  
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
                InstanceId = [[Client1_4705]],  
                Class = [[Position]],  
                x = 32093.23438,  
                y = -1369.21875,  
                z = -27.125
              }
            }
          }
        },  
        {
          InstanceId = [[Client1_4593]],  
          Class = [[NpcGrpFeature]],  
          InheritPos = 1,  
          Name = [[Group 21]],  
          ActivitiesId = {
          },  
          Behavior = {
            InstanceId = [[Client1_4591]],  
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
              InstanceId = [[Client1_4579]],  
              Class = [[NpcCreature]],  
              Angle = 2.369272709,  
              Base = [[palette.entities.creatures.chhfd3]],  
              InheritPos = 1,  
              Name = [[Domestic Mektoub]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4577]],  
                Class = [[Behavior]],  
                Type = [[]],  
                ZoneId = [[]],  
                Actions = {
                },  
                Activities = {
                  {
                    InstanceId = [[Client1_4688]],  
                    Class = [[ActivitySequence]],  
                    Name = [[]],  
                    Repeating = 1,  
                    Components = {
                      {
                        InstanceId = [[Client1_4689]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Rest In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_4669]]),  
                        Chat = r2.RefId([[]]),  
                        Name = [[]],  
                        TimeLimit = [[Few Sec]],  
                        TimeLimitValue = [[20]],  
                        Type = [[None]],  
                        EventsIds = {
                        }
                      },  
                      {
                        InstanceId = [[Client1_4690]],  
                        Class = [[ActivityStep]],  
                        Activity = [[Feed In Zone]],  
                        ActivityZoneId = r2.RefId([[Client1_4669]]),  
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
                InstanceId = [[Client1_4580]],  
                Class = [[Position]],  
                x = 32100.01563,  
                y = -1381.546875,  
                z = -27.890625
              }
            },  
            {
              InstanceId = [[Client1_4588]],  
              Class = [[NpcCreature]],  
              Angle = 2.369272709,  
              Base = [[palette.entities.creatures.chhfd3]],  
              InheritPos = 1,  
              Name = [[Domestic Mektoub]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4589]],  
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
                InstanceId = [[Client1_4590]],  
                Class = [[Position]],  
                x = 32096.71875,  
                y = -1385.65625,  
                z = -27.5
              }
            },  
            {
              InstanceId = [[Client1_4601]],  
              Class = [[NpcCreature]],  
              Angle = 2.369272709,  
              Base = [[palette.entities.creatures.chhfd3]],  
              InheritPos = 1,  
              Name = [[Domestic Mektoub]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4602]],  
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
                InstanceId = [[Client1_4604]],  
                Class = [[Position]],  
                x = 32109.54688,  
                y = -1388.5,  
                z = -26.375
              }
            },  
            {
              InstanceId = [[Client1_4612]],  
              Class = [[NpcCreature]],  
              Angle = 2.369272709,  
              Base = [[palette.entities.creatures.chhfd3]],  
              InheritPos = 1,  
              Name = [[Domestic Mektoub]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4613]],  
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
                InstanceId = [[Client1_4615]],  
                Class = [[Position]],  
                x = 32097.42188,  
                y = -1392.1875,  
                z = -26.875
              }
            },  
            {
              InstanceId = [[Client1_4623]],  
              Class = [[NpcCreature]],  
              Angle = 2.369272709,  
              Base = [[palette.entities.creatures.chhfd3]],  
              InheritPos = 1,  
              Name = [[Domestic Mektoub]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4624]],  
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
                InstanceId = [[Client1_4626]],  
                Class = [[Position]],  
                x = 32113,  
                y = -1394.703125,  
                z = -25.578125
              }
            },  
            {
              InstanceId = [[Client1_4664]],  
              Class = [[NpcCreature]],  
              Angle = 2.369272709,  
              Base = [[palette.entities.creatures.chhfd3]],  
              InheritPos = 1,  
              Name = [[Domestic Mektoub]],  
              ActivitiesId = {
              },  
              Behavior = {
                InstanceId = [[Client1_4665]],  
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
                InstanceId = [[Client1_4667]],  
                Class = [[Position]],  
                x = 32104.875,  
                y = -1391.8125,  
                z = -26.515625
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_4592]],  
            Class = [[Position]],  
            x = 0,  
            y = 0,  
            z = 0
          }
        },  
        {
          InstanceId = [[Client1_4723]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 1,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Act 4 start]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_4721]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_4724]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_4725]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_4728]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            },  
            {
              InstanceId = [[Client1_4726]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_4727]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_4731]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            },  
            {
              InstanceId = [[Client1_4729]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_4730]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_4732]],  
                  Who = r2.RefId([[]]),  
                  WhoNoEntity = [[_System]]
                }
              }
            },  
            {
              InstanceId = [[Client1_4733]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_4734]],  
                  Class = [[ChatAction]],  
                  Emote = [[Cheer]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_4735]],  
                  Who = r2.RefId([[Client1_4693]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_4736]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 1,  
              Actions = {
                {
                  InstanceId = [[Client1_4737]],  
                  Class = [[ChatAction]],  
                  Emote = [[Calmdown]],  
                  Facing = r2.RefId([[Client1_4693]]),  
                  Says = [[Client1_4739]],  
                  Who = r2.RefId([[Client1_4697]]),  
                  WhoNoEntity = [[]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_4722]],  
            Class = [[Position]],  
            x = 32061.85938,  
            y = -1386.015625,  
            z = -27
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_5818]],  
          Class = [[RewardProvider]],  
          Active = 0,  
          Base = [[palette.entities.botobjects.bot_chat]],  
          ContextualText = [[Talk to <reward_giver>]],  
          InheritPos = 1,  
          InventoryFullText = [[Kami no more gifts can give because Homin no more can carry]],  
          Name = [[Reward Provider 1]],  
          NotEnoughPointsText = [[Kami give blessing to homin.]],  
          OnTargetText = [[Kami think rewards deserved for helping farmer.]],  
          RareRewardText = [[Kami give homin extra special gift!]],  
          Repeatable = 1,  
          RewardGiver = r2.RefId([[Client1_4575]]),  
          RewardText = [[Kami give homin gifts]],  
          _Seed = 1161015977,  
          Behavior = {
            InstanceId = [[Client1_5819]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
          },  
          Position = {
            InstanceId = [[Client1_5820]],  
            Class = [[Position]],  
            x = 32101.53125,  
            y = -1373.484375,  
            z = -27.546875
          }
        },  
        {
          InstanceId = [[Client1_4742]],  
          Class = [[ChatSequence]],  
          Active = 1,  
          AutoStart = 0,  
          Base = [[palette.entities.botobjects.dialog]],  
          InheritPos = 1,  
          Name = [[Kami]],  
          Repeating = 0,  
          Type = [[None]],  
          Behavior = {
            InstanceId = [[Client1_4740]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
              {
                InstanceId = [[Client1_5822]],  
                Class = [[LogicEntityAction]],  
                Name = [[]],  
                Actions = {
                  {
                    InstanceId = [[Client1_5825]],  
                    Class = [[ActionStep]],  
                    Entity = r2.RefId([[Client1_5818]]),  
                    Action = {
                      InstanceId = [[Client1_5824]],  
                      Class = [[ActionType]],  
                      Type = [[activate]],  
                      Value = r2.RefId([[]])
                    }
                  }
                },  
                Conditions = {
                },  
                Event = {
                  InstanceId = [[Client1_5823]],  
                  Class = [[EventType]],  
                  Type = [[end of dialog]],  
                  Value = r2.RefId([[]])
                }
              }
            }
          },  
          Components = {
            {
              InstanceId = [[Client1_4743]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 0,  
              Actions = {
                {
                  InstanceId = [[Client1_4744]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_4747]],  
                  Who = r2.RefId([[Client1_4575]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_4745]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_4746]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_4750]],  
                  Who = r2.RefId([[Client1_4575]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_4748]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 1,  
              Actions = {
                {
                  InstanceId = [[Client1_4749]],  
                  Class = [[ChatAction]],  
                  Emote = [[Bow]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_4751]],  
                  Who = r2.RefId([[Client1_4697]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_4752]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 2,  
              Actions = {
                {
                  InstanceId = [[Client1_4753]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_4756]],  
                  Who = r2.RefId([[Client1_4575]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_4758]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_4759]],  
                  Class = [[ChatAction]],  
                  Emote = [[Bow]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_4760]],  
                  Who = r2.RefId([[Client1_4708]]),  
                  WhoNoEntity = [[]]
                }
              }
            },  
            {
              InstanceId = [[Client1_4754]],  
              Class = [[ChatStep]],  
              Name = [[]],  
              Time = 3,  
              Actions = {
                {
                  InstanceId = [[Client1_4755]],  
                  Class = [[ChatAction]],  
                  Emote = [[]],  
                  Facing = r2.RefId([[]]),  
                  Says = [[Client1_4761]],  
                  Who = r2.RefId([[Client1_4575]]),  
                  WhoNoEntity = [[]]
                }
              }
            }
          },  
          Position = {
            InstanceId = [[Client1_4741]],  
            Class = [[Position]],  
            x = 32099.90625,  
            y = -1376.390625,  
            z = -27
          },  
          SubComponents = {
          }
        },  
        {
          InstanceId = [[Client1_5844]],  
          Class = [[RewardProvider]],  
          Active = 1,  
          Base = [[palette.entities.botobjects.bot_chat]],  
          ContextualText = [[Talk to <reward_giver>]],  
          InheritPos = 1,  
          InventoryFullText = [[You deserve more but I see that your bag is already full]],  
          Name = [[Reward Provider 2]],  
          NotEnoughPointsText = [[It was a pleasure knowing you]],  
          OnTargetText = [[We have gifts for all of our brave saviours]],  
          RareRewardText = [[Please accept this extra special gift as thanks!]],  
          Repeatable = 1,  
          RewardGiver = r2.RefId([[Client1_4693]]),  
          RewardText = [[Please accept this gift as thanks]],  
          _Seed = 1161018080,  
          Behavior = {
            InstanceId = [[Client1_5845]],  
            Class = [[LogicEntityBehavior]],  
            Actions = {
            }
          },  
          Components = {
          },  
          Position = {
            InstanceId = [[Client1_5846]],  
            Class = [[Position]],  
            x = 32092.1875,  
            y = -1379.03125,  
            z = -27.328125
          }
        }
      },  
      Position = {
        InstanceId = [[Client1_4557]],  
        Class = [[Position]],  
        x = 0,  
        y = 0,  
        z = 0
      }
    }
  },  
  Behavior = {
    InstanceId = [[Client1_3]],  
    Class = [[LogicEntityBehavior]],  
    Actions = {
    }
  },  
  Description = {
    InstanceId = [[Client1_1]],  
    Class = [[MapDescription]],  
    LevelId = 4,  
    OptimalNumberOfPlayer = 0,  
    ShortDescription = [[A farmer's desperate flee from the Kitin, help him get to safety!]],  
    Title = [[Trigio's Plight]]
  },  
  Locations = {
    {
      InstanceId = [[Client1_14]],  
      Class = [[Location]],  
      EntryPoint = [[uiR2EntryPoint02]],  
      IslandName = [[uiR2_Forest35]],  
      ManualSeason = 1,  
      Name = [[Nidera Farm]],  
      Season = [[Spring]],  
      ShortDescription = [[]],  
      Time = 0
    },  
    {
      InstanceId = [[Client1_893]],  
      Class = [[Location]],  
      EntryPoint = [[uiR2EntryPoint03]],  
      IslandName = [[uiR2_Primes03]],  
      ManualSeason = 1,  
      Name = [[Green-cave]],  
      Season = [[Winter]],  
      ShortDescription = [[]],  
      Time = 0
    },  
    {
      InstanceId = [[Client1_2635]],  
      Class = [[Location]],  
      EntryPoint = [[uiR2EntryPoint01]],  
      IslandName = [[uiR2_Lakes07]],  
      ManualSeason = 1,  
      Name = [[Dunes of Dawn]],  
      Season = [[Summer]],  
      ShortDescription = [[]],  
      Time = 0
    },  
    {
      InstanceId = [[Client1_4560]],  
      Class = [[Location]],  
      EntryPoint = [[uiR2EntryPoint01]],  
      IslandName = [[uiR2_Lakes02]],  
      ManualSeason = 1,  
      Name = [[Hidden Archipelago]],  
      Season = [[Autumn]],  
      ShortDescription = [[]],  
      Time = 0
    }
  },  
  PlotItems = {
    {
      InstanceId = [[Client1_39]],  
      Class = [[PlotItem]],  
      Comment = [[Food for Trigio's Mektoubs]],  
      Desc = [[Mektoub Food]],  
      Name = [[Mektoub Food]],  
      SheetId = 8669486
    }
  },  
  Position = {
    InstanceId = [[Client1_4]],  
    Class = [[Position]],  
    x = 0,  
    y = 0,  
    z = 0
  },  
  Texts = {
    InstanceId = [[Client1_2]],  
    Class = [[TextManager]],  
    Texts = {
      {
        InstanceId = [[Client1_821]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Congratulations, mission completed!]]
      },  
      {
        InstanceId = [[Client1_846]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Farmer Trigio Nidera requires your aid, you should talk to him,]]
      },  
      {
        InstanceId = [[Client1_1027]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[You have reached the prime roots after many days walk.]]
      },  
      {
        InstanceId = [[Client1_1028]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Go and speak to Trigio Nidera.]]
      },  
      {
        InstanceId = [[Client1_1678]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Trigio had to let his remaining herd free, it was not safe for them on the remaining journey.]]
      },  
      {
        InstanceId = [[Client1_2156]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Well done, you have successfully reached the camp.]]
      },  
      {
        InstanceId = [[Client1_2157]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[You should report to Be-ci Kuani, the camp leader.]]
      },  
      {
        InstanceId = [[Client1_2514]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Sir!  The Kitin are attacking!]]
      },  
      {
        InstanceId = [[Client1_2515]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[What?!  To the defences!]]
      },  
      {
        InstanceId = [[Client1_2525]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Welcome to the camp, I see Gale and Abyrixius have lead you here well.  We have much left to do before we can move onto safer ground, I shall explain...]]
      },  
      {
        InstanceId = [[Client1_2531]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Well done!  The Kitin have been forced back for now.  It is now time to clear the road ahead of us.]]
      },  
      {
        InstanceId = [[Client1_2536]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Our destination is the Ruins of Silan, where the Atys Rangers have set up a safe base of operations.  We shall be welcome there, but there is an obstacle in our path...]]
      },  
      {
        InstanceId = [[Client1_2539]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[A Tribe of Cut-throats have killed many of the scouts we have sent, we must clear them from our road if we are to continue.]]
      },  
      {
        InstanceId = [[Client1_2542]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[We have a small force waiting at the destination ready for the attack. I shall go with yourself, Abyrixius and Gale to lead the strike.]]
      },  
      {
        InstanceId = [[Client1_2543]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Let us go!]]
      },  
      {
        InstanceId = [[Client1_2560]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Congratulations!  You have successfully completed this Act.]]
      },  
      {
        InstanceId = [[Client1_2648]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[The Kitin are attacking the camp, assist the guards in holding off the Kitin!]]
      },  
      {
        InstanceId = [[Client1_2764]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[You have reached your destination after half a days walk.]]
      },  
      {
        InstanceId = [[Client1_2767]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[There is a bandit camp in the South East blocking the road ahead.  They need to be taken out.]]
      },  
      {
        InstanceId = [[Client1_2768]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Speak to Be-ci Kuani to start.]]
      },  
      {
        InstanceId = [[Client1_2969]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Congratulations, the bandits have been destroyed!]]
      },  
      {
        InstanceId = [[Client1_2970]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[The road is now clear to move onto the next Act!]]
      },  
      {
        InstanceId = [[Client1_4728]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[After two days trek, your band of travellers finally made it to this land.]]
      },  
      {
        InstanceId = [[Client1_4731]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[You have completed this scenario, the Kami of the Lake will fill you in if you wish.]]
      },  
      {
        InstanceId = [[Client1_4732]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Congratulations and well played homin!]]
      },  
      {
        InstanceId = [[Client1_4735]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Oh my Mektoubs are here!  Thank you so much Mr Kami!]]
      },  
      {
        InstanceId = [[Client1_4739]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Calm down dear.]]
      },  
      {
        InstanceId = [[Client1_4747]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Kami hear what homin do.  Kami very proud of homin.]]
      },  
      {
        InstanceId = [[Client1_4750]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Kami find farmer's mektoub, bring to land here.  Farmer very happy.]]
      },  
      {
        InstanceId = [[Client1_4751]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Thank you so much friend!]]
      },  
      {
        InstanceId = [[Client1_4756]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Kami know Be-ci Kuani, he tell all you do.  Kami proud.  Homins go Silan now, with Rangers.]]
      },  
      {
        InstanceId = [[Client1_4760]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Thank you friend, your assistance was most valuable.]]
      },  
      {
        InstanceId = [[Client1_4761]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Be-ci Kuani say not possible without homin you.  Kami remember you now.]]
      },  
      {
        InstanceId = [[Client1_4778]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[Everyone is in position, you should talk to Be-ci Kuani again.]]
      },  
      {
        InstanceId = [[Client1_5784]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[One of your NPCs has been killed!]]
      },  
      {
        InstanceId = [[Client1_5785]],  
        Class = [[TextManagerEntry]],  
        Count = 1,  
        Text = [[You should return to the starting place to try the mission again.]]
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
    ChatAction = 0,  
    ChatSequence = 1,  
    ChatStep = 0,  
    ConditionStep = 0,  
    ConditionType = 0,  
    DefaultFeature = 0,  
    EasterEgg = 1,  
    EventType = 0,  
    Location = 1,  
    LogicEntityAction = 0,  
    LogicEntityBehavior = 1,  
    MapDescription = 0,  
    Npc = 1,  
    NpcCreature = 1,  
    NpcCustom = 1,  
    NpcGrpFeature = 1,  
    PlotItem = 0,  
    Position = 0,  
    Region = 1,  
    RegionVertex = 1,  
    RequestItem = 1,  
    RewardProvider = 1,  
    Road = 1,  
    Scenario = 4,  
    TalkTo = 1,  
    TextManager = 0,  
    TextManagerEntry = 0,  
    Timer = 1,  
    WayPoint = 1,  
    ZoneTrigger = 1
  }
}