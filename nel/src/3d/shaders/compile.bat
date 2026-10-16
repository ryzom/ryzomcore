cgc -entry fxaa_pp fxaa_pp.cg -profile arbfp1 -O3 -fastmath -fastprecision -o fxaa_pp_arbfp1.txt
cgc -entry fxaa_pp fxaa_pp.cg -profile ps_2_x -O3 -fastmath -fastprecision -o fxaa_pp_ps_2_0.txt
cgc -entry fxaa_vp fxaa_vp.cg -profile arbvp1 -fastmath -fastprecision -o fxaa_vp_arbvp1.txt
cgc -entry stereo_debug_pp stereo_debug_pp.cg -profile arbfp1 -O3 -fastmath -fastprecision -o stereo_debug_pp_arbfp1.txt
cgc -entry stereo_debug_pp stereo_debug_pp.cg -profile ps_2_0 -O3 -fastmath -fastprecision -o stereo_debug_pp_ps_2_0.txt
cgc -entry water_fp water_fp.cg -profile arbfp1 -O3 -fastmath -fastprecision -o water_fp_arbfp1.txt
cgc -entry water_fp water_fp.cg -profile arbfp1 -O3 -fastmath -fastprecision -DUSE_FOG -o water_fp_fog_arbfp1.txt
cgc -entry water_fp water_fp.cg -profile arbfp1 -O3 -fastmath -fastprecision -DUSE_DIFFUSE -o water_fp_diffuse_arbfp1.txt
cgc -entry water_fp water_fp.cg -profile arbfp1 -O3 -fastmath -fastprecision -DUSE_DIFFUSE -DUSE_FOG -o water_fp_diffuse_fog_arbfp1.txt
cgc -entry water_fp water_fp.cg -profile ps_2_0 -O3 -fastmath -fastprecision -o water_fp_ps_2_0.txt
cgc -entry water_fp water_fp.cg -profile ps_2_0 -O3 -fastmath -fastprecision -DUSE_DIFFUSE -o water_fp_diffuse_ps_2_0.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile arbvp1 -O3 -DNUM_POINT_LIGHTS=0 -o wind_tree_vp_0pl_arbvp1.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile arbvp1 -O3 -DNUM_POINT_LIGHTS=0 -DUSE_NORMALIZE -o wind_tree_vp_0pl_norm_arbvp1.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile arbvp1 -O3 -DNUM_POINT_LIGHTS=0 -DUSE_SPECULAR -o wind_tree_vp_0pl_spec_arbvp1.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile arbvp1 -O3 -DNUM_POINT_LIGHTS=0 -DUSE_SPECULAR -DUSE_NORMALIZE -o wind_tree_vp_0pl_spec_norm_arbvp1.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile arbvp1 -O3 -DNUM_POINT_LIGHTS=1 -o wind_tree_vp_1pl_arbvp1.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile arbvp1 -O3 -DNUM_POINT_LIGHTS=1 -DUSE_NORMALIZE -o wind_tree_vp_1pl_norm_arbvp1.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile arbvp1 -O3 -DNUM_POINT_LIGHTS=1 -DUSE_SPECULAR -o wind_tree_vp_1pl_spec_arbvp1.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile arbvp1 -O3 -DNUM_POINT_LIGHTS=1 -DUSE_SPECULAR -DUSE_NORMALIZE -o wind_tree_vp_1pl_spec_norm_arbvp1.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile arbvp1 -O3 -DNUM_POINT_LIGHTS=2 -o wind_tree_vp_2pl_arbvp1.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile arbvp1 -O3 -DNUM_POINT_LIGHTS=2 -DUSE_NORMALIZE -o wind_tree_vp_2pl_norm_arbvp1.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile arbvp1 -O3 -DNUM_POINT_LIGHTS=2 -DUSE_SPECULAR -o wind_tree_vp_2pl_spec_arbvp1.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile arbvp1 -O3 -DNUM_POINT_LIGHTS=2 -DUSE_SPECULAR -DUSE_NORMALIZE -o wind_tree_vp_2pl_spec_norm_arbvp1.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile arbvp1 -O3 -DNUM_POINT_LIGHTS=3 -o wind_tree_vp_3pl_arbvp1.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile arbvp1 -O3 -DNUM_POINT_LIGHTS=3 -DUSE_NORMALIZE -o wind_tree_vp_3pl_norm_arbvp1.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile arbvp1 -O3 -DNUM_POINT_LIGHTS=3 -DUSE_SPECULAR -o wind_tree_vp_3pl_spec_arbvp1.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile arbvp1 -O3 -DNUM_POINT_LIGHTS=3 -DUSE_SPECULAR -DUSE_NORMALIZE -o wind_tree_vp_3pl_spec_norm_arbvp1.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile vs_2_0 -O3 -DNUM_POINT_LIGHTS=0 -o wind_tree_vp_0pl_vs20.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile vs_2_0 -O3 -DNUM_POINT_LIGHTS=0 -DUSE_NORMALIZE -o wind_tree_vp_0pl_norm_vs20.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile vs_2_0 -O3 -DNUM_POINT_LIGHTS=0 -DUSE_SPECULAR -o wind_tree_vp_0pl_spec_vs20.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile vs_2_0 -O3 -DNUM_POINT_LIGHTS=0 -DUSE_SPECULAR -DUSE_NORMALIZE -o wind_tree_vp_0pl_spec_norm_vs20.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile vs_2_0 -O3 -DNUM_POINT_LIGHTS=1 -o wind_tree_vp_1pl_vs20.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile vs_2_0 -O3 -DNUM_POINT_LIGHTS=1 -DUSE_NORMALIZE -o wind_tree_vp_1pl_norm_vs20.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile vs_2_0 -O3 -DNUM_POINT_LIGHTS=1 -DUSE_SPECULAR -o wind_tree_vp_1pl_spec_vs20.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile vs_2_0 -O3 -DNUM_POINT_LIGHTS=1 -DUSE_SPECULAR -DUSE_NORMALIZE -o wind_tree_vp_1pl_spec_norm_vs20.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile vs_2_0 -O3 -DNUM_POINT_LIGHTS=2 -o wind_tree_vp_2pl_vs20.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile vs_2_0 -O3 -DNUM_POINT_LIGHTS=2 -DUSE_NORMALIZE -o wind_tree_vp_2pl_norm_vs20.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile vs_2_0 -O3 -DNUM_POINT_LIGHTS=2 -DUSE_SPECULAR -o wind_tree_vp_2pl_spec_vs20.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile vs_2_0 -O3 -DNUM_POINT_LIGHTS=2 -DUSE_SPECULAR -DUSE_NORMALIZE -o wind_tree_vp_2pl_spec_norm_vs20.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile vs_2_0 -O3 -DNUM_POINT_LIGHTS=3 -o wind_tree_vp_3pl_vs20.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile vs_2_0 -O3 -DNUM_POINT_LIGHTS=3 -DUSE_NORMALIZE -o wind_tree_vp_3pl_norm_vs20.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile vs_2_0 -O3 -DNUM_POINT_LIGHTS=3 -DUSE_SPECULAR -o wind_tree_vp_3pl_spec_vs20.txt
cgc -entry wind_tree_vp wind_tree_vp.cg -profile vs_2_0 -O3 -DNUM_POINT_LIGHTS=3 -DUSE_SPECULAR -DUSE_NORMALIZE -o wind_tree_vp_3pl_spec_norm_vs20.txt