#include "../include/triangle.h"
#include "../include/display.h"
#include <math.h>
#include "../include/swap.h"
#include <dc/perf_monitor.h>
#include "../include/utils.h"
void draw_filled_triangle( 
    int x0, int y0, float z0, float w0,
    int x1, int y1, float z1, float w1,
    int x2, int y2, float z2, float w2, uint16_t color){
   

     if(y0 > y1){
        int_swap(&y0,&y1);
        int_swap(&x0,&x1);

        float_swap(&z0,&z1);
        float_swap(&w0,&w1);


       
    }
    if(y1 > y2){
        int_swap(&y1,&y2);
        int_swap(&x1,&x2);

        float_swap(&z1,&z2);
        float_swap(&w1,&w2);

    }
    if ( y0 > y1){
        int_swap(&y0,&y1);
        int_swap(&x0,&x1);

        float_swap(&z0,&z1);
        float_swap(&w0,&w1);

      
    }

  
    //Create vector points after sorting vertices of triangle
    vector_t point_a = {x0,y0,z0,w0};
    vector_t point_b = {x1,y1,z1,w1};
    vector_t point_c = {x2,y2,z2,w2};


    float inv_slope_1 = 0;
    float inv_slope_2 = 0;

    //inv_slope_1 = dx/dy => (x1-x0)/(y1-y0)
    //inv_slope_2 = dx/dy => (x2-x0)/(y2-y0)

    if(y1 - y0 != 0)inv_slope_1 = (float)(x1-x0)/fabs(y1-y0);
    if(y2 - y0 != 0)inv_slope_2 = (float)(x2-x0)/fabs(y2-y0);

    
    ////////////////////////////
    // Draw flat bottom triangle
    ///////////////////////////


    if(y1-y0 != 0){
        for(int y = y0; y <= y1; y ++){
            
            int x_start = x1 + (y-y1) * inv_slope_1;
            int x_end = x0 + (y-y0) * inv_slope_2;

            if(x_end < x_start){
                int_swap(&x_start,&x_end); // if a triangle is rotated a certain way x_start could be greater than x_end
            }

            for(int x = x_start; x < x_end; x++){
                
               draw_filled_pixel(x, y, point_a,  point_b, point_c, color);
            }
        }
    }


    ////////////////////////////
    // Draw flat top triangle
    ///////////////////////////


    inv_slope_1 = 0;
    inv_slope_2 = 0;

    //inv_slope_1 = dx/dy => (x2-x1)/(y1-y0)
    //inv_slope_2 = dx/dy => (x2-x0)/(y2-y0)

    if(y2 - y1 != 0)inv_slope_1 = (float)(x2-x1)/fabs(y2-y1);
    if(y2 - y0 != 0)inv_slope_2 = (float)(x2-x0)/fabs(y2-y0);

    if(y2-y1 != 0){
        for(int y = y1; y <= y2; y ++){
            
            int x_start = x1 + (y-y1) * inv_slope_1;
            int x_end   = x0 + (y-y0) * inv_slope_2;

            if(x_end < x_start){
                int_swap(&x_start,&x_end); // if a triangle is rotated a certain way x_start could be greater than x_end
            }

            for(int x = x_start; x < x_end; x++){
                
               draw_filled_pixel(x, y, point_a,  point_b, point_c, color);
            }
        }
    }

}




void draw_textured_triangle(
    int x0, int y0, float z0, float w0, float u0, float v0,
    int x1, int y1, float z1, float w1, float u1, float v1,
    int x2, int y2, float z2, float w2, float u2, float v2,
    kos_img_t texture
){


   if(y0 > y1){
        int_swap(&y0,&y1);
        int_swap(&x0,&x1);

        float_swap(&z0,&z1);
        float_swap(&w0,&w1);

        float_swap(&u0,&u1);
        float_swap(&v0,&v1);
    }
    if(y1 > y2){
        int_swap(&y1,&y2);
        int_swap(&x1,&x2);

        float_swap(&z1,&z2);
        float_swap(&w1,&w2);

        float_swap(&u1,&u2);
        float_swap(&v1,&v2);
    }
    if ( y0 > y1){
        int_swap(&y0,&y1);
        int_swap(&x0,&x1);

        float_swap(&z0,&z1);
        float_swap(&w0,&w1);

        float_swap(&u0,&u1);
        float_swap(&v0,&v1);
    }

    //Flip the V component for inverted V coordinates
    v0 = 1.0-v0;
    v1 = 1.0-v1;
    v2 = 1.0-v2;

    //Create vector points after sorting vertices of triangle
    vector_t point_a = {x0,y0,z0,w0};
    vector_t point_b = {x1,y1,z1,w1};
    vector_t point_c = {x2,y2,z2,w2};


    float inv_slope_1 = 0;
    float inv_slope_2 = 0;

    //inv_slope_1 = dx/dy => (x1-x0)/(y1-y0)
    //inv_slope_2 = dx/dy => (x2-x0)/(y2-y0)

    if(y1 - y0 != 0)inv_slope_1 = (float)(x1-x0)/fabs(y1-y0);
    if(y2 - y0 != 0)inv_slope_2 = (float)(x2-x0)/fabs(y2-y0);

    
    ////////////////////////////
    // Draw flat bottom triangle
    ///////////////////////////


    if(y1-y0 != 0){
        for(int y = y0; y <= y1; y ++){
            
            int x_start = x1 + (y-y1) * inv_slope_1;
            int x_end = x0 + (y-y0) * inv_slope_2;

            if(x_end < x_start){
                int_swap(&x_start,&x_end); // if a triangle is rotated a certain way x_start could be greater than x_end
            }

            for(int x = x_start; x < x_end; x++){
                
                draw_textured_texel(x, y, point_a,  point_b, point_c, u0, u1, v0, v1, u2, v2, texture);
            }
        }
    }


    ////////////////////////////
    // Draw flat top triangle
    ///////////////////////////


    inv_slope_1 = 0;
    inv_slope_2 = 0;

    //inv_slope_1 = dx/dy => (x2-x1)/(y1-y0)
    //inv_slope_2 = dx/dy => (x2-x0)/(y2-y0)

    if(y2 - y1 != 0)inv_slope_1 = (float)(x2-x1)/fabs(y2-y1);
    if(y2 - y0 != 0)inv_slope_2 = (float)(x2-x0)/fabs(y2-y0);

    if(y2-y1 != 0){
        for(int y = y1; y <= y2; y ++){
            
            int x_start = x1 + (y-y1) * inv_slope_1;
            int x_end   = x0 + (y-y0) * inv_slope_2;

            if(x_end < x_start){
                int_swap(&x_start,&x_end); // if a triangle is rotated a certain way x_start could be greater than x_end
            }

            for(int x = x_start; x < x_end; x++){
                
                draw_textured_texel(x, y, point_a,  point_b, point_c, u0, u1, v0, v1, u2, v2, texture);
            }
        }
    }
}

void draw_filled_pixel(int x, int y, vector_t point_a, vector_t point_b, vector_t point_c, uint16_t color){
    vec2_t p = {x,y};

    vec2_t a = vec2_from_vec4(point_a);
    vec2_t b = vec2_from_vec4(point_b);
    vec2_t c = vec2_from_vec4(point_c);

    vec3f_t weights = barycentric_weights(a,b,c,p);
    
    float alpha = weights.x;
    float beta  = weights.y;
    float gamma = weights.z;

    // interpolated reciprocal of W
    float interpolated_reciprocal_w = (1 / point_a.w) * alpha + (1 / point_b.w) * beta + (1 / point_c.w) * gamma;


    //Only draw pixel if depth of pixel is greater than previous pixel
    // 1 <- right in front of camera 0 -> farthest point to the camera

    if(interpolated_reciprocal_w > get_z_buffer_at(x, y)){
        draw_pixel(x,y,color);
        //update z buffer of current pixel
        update_zbuffer(x, y, interpolated_reciprocal_w);
    }
}

vec3f_t get_triangle_face_normal(vector_t vertices[3]){
    vec3f_t v1 = vec3_sub(vec3_from_vec4(vertices[1]),vec3_from_vec4(vertices[0]));
    vec3f_t v2 = vec3_sub(vec3_from_vec4(vertices[2]),vec3_from_vec4(vertices[0]));
    vec3f_t tri_normal = vec3_cross(v1,v2);

    return tri_normal;
}

void draw_textured_texel(int x, int y, vector_t point_a, vector_t point_b, vector_t point_c, float u0, float u1, float v0, float v1, float u2, float v2, kos_img_t texture){

    vec2_t p = {x,y};

    vec2_t a = vec2_from_vec4(point_a);
    vec2_t b = vec2_from_vec4(point_b);
    vec2_t c = vec2_from_vec4(point_c);

    vec3f_t weights = barycentric_weights(a,b,c,p);
    
    float alpha = weights.x;
    float beta  = weights.y;
    float gamma = weights.z;

    // interpolated values of U and V and the reciprocal of W
    float interpolated_u;
    float interpolated_v;
    float interpolated_reciprocal_w;

    interpolated_u = (u0 / point_a.w) * alpha + (u1 / point_b.w) * beta + (u2 / point_c.w) * gamma;
    interpolated_v = (v0 / point_a.w) * alpha + (v1 / point_b.w) * beta + (v2 / point_c.w) * gamma;


    // Also interpolate the value of 1/w for the current pixel
    interpolated_reciprocal_w = ( 1 / point_a.w ) * alpha + ( 1 / point_b.w ) * beta + ( 1 / point_c.w ) * gamma;

    // Now we divide back both values by 1/W
    interpolated_u /= interpolated_reciprocal_w;
    interpolated_v /= interpolated_reciprocal_w;

    interpolated_u = fminf(fmaxf(interpolated_u, 0.0f), 1.0f);
    interpolated_v = fminf(fmaxf(interpolated_v, 0.0f), 1.0f);

    // Get texture width and height dimensions
    int texture_width = (texture.w);
    int texture_height = (texture.h);

    int tex_x = (int)(interpolated_u * (texture_width - 1));
    int tex_y = (int)(interpolated_v * (texture_height - 1));
    
    //Only draw pixel if depth of pixel is greater than previous pixel
    // 1 <- right in front of camera 0 -> farthest point to the camera
    
    if(interpolated_reciprocal_w > get_z_buffer_at(x, y) ){
        uint16_t* tex_data = (uint16_t*)texture.data;
        uint16_t color = tex_data[tex_x + (tex_y * texture_width)];
        draw_pixel(x,y,color);
        
        //update z buffer of current pixel
        update_zbuffer(x, y, interpolated_reciprocal_w);
       // printf("updating z buffer tex_x: %d, tex_y: %d, color: %d\n", tex_x, tex_y, color);
    }
}

vec3f_t barycentric_weights(vec2_t a, vec2_t b, vec2_t c, vec2_t p){
    vec2_t ac = vec2_sub(c, a);
    vec2_t ab = vec2_sub(b, a);
    vec2_t ap = vec2_sub(p, a);
    vec2_t pc = vec2_sub(c, p);
    vec2_t pb = vec2_sub(b, p);

    float area_of_abc = (ac.x * ab.y - ac.y * ab.x); //  || AC X AB  ||

    float alpha = (pc.x * pb.y - pc.y * pb.x) / area_of_abc;

    float beta = (ac.x * ap.y - ac.y * ap.x) / area_of_abc;

    float gamma = 1.0 - alpha - beta;

    vec3f_t weights = {alpha, beta, gamma};

    return weights;
    
}
