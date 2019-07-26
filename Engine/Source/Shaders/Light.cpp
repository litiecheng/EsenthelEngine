/******************************************************************************/
#include "!Header.h"
BUFFER(LightMap)
   Flt LightMapScale=1;
BUFFER_END
/******************************************************************************/
// Img=Nrm (this also used for Water Apply shader), ImgMS=NrmMS, Img1=ConeLight.Lightmap, ImgX=shadow
/******************************************************************************/
VecH4 LightDir_PS(NOPERSP Vec2 inTex  :TEXCOORD0,
                  NOPERSP Vec2 inPosXY:TEXCOORD1
               #if MULTI_SAMPLE
                , NOPERSP PIXEL                      
                ,         UInt index  :SV_SampleIndex
               #endif
                  ):TARGET
{
   // shadow
#if MULTI_SAMPLE
   Half shd; if(SHADOW)shd=TexSample(ImgXMS, pixel.xy, index).x;
#else
   Half shd; if(SHADOW)shd=TexPoint(ImgX, inTex).x;
#endif

   // normal
#if MULTI_SAMPLE
   VecH4 nrm=GetNormalMS(pixel.xy, index, QUALITY);
#else
   VecH4 nrm=GetNormal(inTex, QUALITY);
#endif

   // diffuse
   Half lum=LightDiffuse(nrm.xyz, LightDir.dir); if(SHADOW)lum*=shd; clip(lum-EPS_LUM);

   // specular
   VecH eye_dir =Normalize    (-Vec(inPosXY, 1));
   Half specular=LightSpecular(nrm.xyz, nrm.w, LightDir.dir, eye_dir); if(SHADOW)specular*=shd;

   return VecH4(LightDir.color.rgb*lum, LightDir.color.a*specular);
}
/******************************************************************************/
VecH4 LightPoint_PS(NOPERSP Vec2 inTex  :TEXCOORD0,
                    NOPERSP Vec2 inPosXY:TEXCOORD1
                 #if MULTI_SAMPLE
                  , NOPERSP PIXEL                      
                  ,         UInt index  :SV_SampleIndex
                 #endif
                   ):TARGET
{
   // shadow
#if MULTI_SAMPLE
   Half shd; if(SHADOW){shd=ShadowFinal(TexSample(ImgXMS, pixel.xy, index).x); clip(shd-EPS_LUM);}
#else
   Half shd; if(SHADOW){shd=ShadowFinal(TexPoint(ImgX, inTex).x); clip(shd-EPS_LUM);}
#endif

   // distance
#if MULTI_SAMPLE
   Vec pos=GetPosMS(pixel.xy, index, inPosXY);
#else
   Vec pos=GetPosPoint(inTex, inPosXY);
#endif
   Vec  delta=LightPoint.pos-pos; Flt inv_dist2=1/Length2(delta);
   Half power=LightPointDist(inv_dist2); if(SHADOW)power*=shd; clip(power-EPS_LUM);

   // normal
#if MULTI_SAMPLE
   VecH4 nrm=GetNormalMS(pixel.xy, index, QUALITY);
#else
   VecH4 nrm=GetNormal(inTex, QUALITY);
#endif

   // diffuse
   VecH light_dir=delta*Sqrt(inv_dist2); // Normalize(delta);
   Half lum      =LightDiffuse(nrm.xyz, light_dir)*power;

   // specular
   VecH eye_dir =Normalize    (-pos);
   Half specular=LightSpecular( nrm.xyz, nrm.w, light_dir, eye_dir)*power;

   return VecH4(LightPoint.color.rgb*lum, LightPoint.color.a*specular);
}
/******************************************************************************/
VecH4 LightLinear_PS(NOPERSP Vec2 inTex  :TEXCOORD0,
                     NOPERSP Vec2 inPosXY:TEXCOORD1
                  #if MULTI_SAMPLE
                   , NOPERSP PIXEL                      
                   ,         UInt index  :SV_SampleIndex
                  #endif
                    ):TARGET
{
   // shadow
#if MULTI_SAMPLE
   Half shd; if(SHADOW){shd=ShadowFinal(TexSample(ImgXMS, pixel.xy, index).x); clip(shd-EPS_LUM);}
#else
   Half shd; if(SHADOW){shd=ShadowFinal(TexPoint(ImgX, inTex).x); clip(shd-EPS_LUM);}
#endif

   // distance
#if MULTI_SAMPLE
   Vec pos=GetPosMS(pixel.xy, index, inPosXY);
#else
   Vec pos=GetPosPoint(inTex, inPosXY);
#endif
   Vec  delta=LightLinear.pos-pos; Flt dist=Length(delta);
   Half power=LightLinearDist(dist); if(SHADOW)power*=shd; clip(power-EPS_LUM);

   // normal
#if MULTI_SAMPLE
   VecH4 nrm=GetNormalMS(pixel.xy, index, QUALITY);
#else
   VecH4 nrm=GetNormal(inTex, QUALITY);
#endif

   // diffuse
   VecH light_dir=delta/dist; // Normalize(delta);
   Half lum      =LightDiffuse(nrm.xyz, light_dir)*power;

   // specular
   VecH eye_dir =Normalize    (-pos);
   Half specular=LightSpecular( nrm.xyz, nrm.w, light_dir, eye_dir)*power;

   return VecH4(LightLinear.color.rgb*lum, LightLinear.color.a*specular);
}
/******************************************************************************/
VecH4 LightCone_PS(NOPERSP Vec2 inTex  :TEXCOORD0,
                   NOPERSP Vec2 inPosXY:TEXCOORD1
                #if MULTI_SAMPLE
                 , NOPERSP PIXEL                      
                 ,         UInt index  :SV_SampleIndex
                #endif
                  ):TARGET
{
   // shadow
#if MULTI_SAMPLE
   Half shd; if(SHADOW){shd=ShadowFinal(TexSample(ImgXMS, pixel.xy, index).x); clip(shd-EPS_LUM);}
#else
   Half shd; if(SHADOW){shd=ShadowFinal(TexPoint(ImgX, inTex).x); clip(shd-EPS_LUM);}
#endif

   // distance & angle
#if MULTI_SAMPLE
   Vec pos=GetPosMS(pixel.xy, index, inPosXY);
#else
   Vec pos=GetPosPoint(inTex, inPosXY);
#endif
   Vec  delta=LightCone.pos-pos,
        dir  =TransformTP(delta, LightCone.mtrx); dir.xy/=dir.z; clip(Vec(1-Abs(dir.xy), dir.z));
   Flt  dist =Length(delta);
   Half power=LightConeAngle(dir.xy)*LightConeDist(dist); if(SHADOW)power*=shd; clip(power-EPS_LUM);

   // normal
#if MULTI_SAMPLE
   VecH4 nrm=GetNormalMS(pixel.xy, index, QUALITY);
#else
   VecH4 nrm=GetNormal(inTex, QUALITY);
#endif

   // diffuse
   VecH light_dir=delta/dist; // Normalize(delta);
   Half lum      =LightDiffuse(nrm.xyz, light_dir)*power;

   // specular
   VecH eye_dir =Normalize    (-pos);
   Half specular=LightSpecular( nrm.xyz, nrm.w, light_dir, eye_dir)*power;

#if IMAGE
   VecH map_col=Tex(Img1, dir.xy*(LightMapScale*0.5)+0.5).rgb;
   return VecH4(LightCone.color.rgb*lum*map_col, LightCone.color.a*specular);
#else
   return VecH4(LightCone.color.rgb*lum, LightCone.color.a*specular);
#endif
}
/******************************************************************************/
