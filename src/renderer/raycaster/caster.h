#pragma once
#include "hittable/object.h"
#include "../scene.h"

Vector3 RayCast(const Ray* r, Scene* world, i32 depth);

// Vector3 RayCast(ray* r, hit_list* list, i32 depth, i32* total_rays)
// {
//     // PROFILE_START;
//     static const v3_f32 sc = {0, 0, -1};
//     static const v3_f32 white = { 1.0f, 1.0f, 1.0f };
//     static const v3_f32 black = { 0.0f, 0.0f, 0.0f };
//     static const v3_f32 blueish = { 0.1f, 0.1f, 0.1f };

//     hit_record rec;


//     if(depth <= 0)
//         return black;
        
//     *total_rays += 1;

//     if(hit_list_hit_all(list, r, 0.001f, 0xFFFFFF, &rec))
//     {
//         ray scattered;

//         switch (rec.m->type)
//         {
//         case MTYPE_LAMBERTIAN:
//             if(ray_scatter_lambertian(r, &rec, &scattered))
//             {
//                 v3_f32 color = texture_get_color_at(&rec.m->texture, rec.uv, rec.p);
//                 return v3_f32_mult(color, ray_color(&scattered, list, depth - 1, total_rays));
//             }
//             break;
//         case MTYPE_METAL:
//             if(ray_scatter_metal(r, &rec, &scattered))
//             {
//                 v3_f32 color = texture_get_color_at(&rec.m->texture, rec.uv, rec.p);
//                 return v3_f32_mult(color, ray_color(&scattered, list, depth - 1, total_rays));
//             }
//             break;
//         case MTYPE_DIELECTRIC:
//             if(ray_scatter_dielectric(r, &rec, &scattered))
//             {
//                 v3_f32 color = texture_get_color_at(&rec.m->texture, rec.uv, rec.p);
//                 return v3_f32_mult(color, ray_color(&scattered, list, depth - 1, total_rays));
//             }
//             break;
//         case MTYPE_DIFF_LIGHT:
//             // TODO: Assume no scatter for now
//             // If it scatters return emitted + attenuation * ray_color(scattered, background, world, depth-1);
//             // Instead
//             switch (rec.m->texture.type)
//             {
//             case TTYPE_COLOR:
//                 return rec.m->texture.color[0];
//             case TTYPE_IMAGE_ALBEDO:
//                 return texture_get_color_at(&rec.m->texture, rec.uv, rec.p);
//             }
//         }
//         return black;
//     }

//     return black;
    
//     // NOTE: This is a return for colored backgrounds with diffuse light
//     // v3_f32 u_dir = v3_f32_to_unit(r->d);
//     // f32 t = 0.5f * (u_dir.y + 1.0f);


//     // return v3_f32_add(v3_f32_scalar_mult(white, (1.0f - t)), v3_f32_scalar_mult(blueish, t));
//     // PROFILE_END;
// }