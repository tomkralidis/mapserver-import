/******************************************************************************
 * $Id: source_mapserver.c 12529 2011-09-07 12:04:00Z tbonfort $
 *
 * Project:  MapServer
 * Purpose:  MapCache tile caching support file: Mapserver Mapfile datasource
 * Author:   Thomas Bonfort and the MapServer team.
 *
 ******************************************************************************
 * Copyright (c) 1996-2011 Regents of the University of Minnesota.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies of this Software or works derived from this Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *****************************************************************************/

#ifdef USE_MAPSERVER

#include "mapcache.h"
#include "ezxml.h"
#include <apr_tables.h>
#include <apr_strings.h>
#include "../../mapserver.h"

/**
 * \private \memberof mapcache_source_mapserver
 * \sa mapcache_source::render_map()
 */
void _mapcache_source_mapserver_render_map(mapcache_context *ctx, mapcache_map *map) {
   mapcache_source_mapserver *mapserver = (mapcache_source_mapserver*)map->tileset->source;
   mapObj *omap = msLoadMap(mapserver->mapfile,NULL);

  
  /* copy the original mapfile's projection to any layer that doesn't already 
  ** have a projection. This will prevent problems when users forget to 
  ** explicitly set a projection on all layers in a WMS mapfile.
  */
   projectionObj newProj;
   msInitProjection(&newProj);
   msLoadProjectionString(&newProj, map->grid_link->grid->srs);
   
   if(msProjectionsDiffer(&(omap->projection), &newProj)) {
      char *original_srs = NULL;
      int i;
      for(i=0; i<omap->numlayers; i++)
      {
         if (GET_LAYER(omap, i)->projection.numargs <= 0 &&
               GET_LAYER(omap, i)->status != MS_OFF &&
               GET_LAYER(omap, i)->transform == MS_TRUE)
         {
            /* This layer is turned on and needs a projection */

            /* Fetch main map projection string only now that we need it */
            if (original_srs == NULL)
               original_srs = msGetProjectionString(&(omap->projection));

            msLoadProjectionString(&(GET_LAYER(omap, i)->projection),original_srs);
            GET_LAYER(omap, i)->project = MS_TRUE;
         }
      }
      msFree(original_srs);
      msLoadProjectionString(&(omap->projection), map->grid_link->grid->srs);
      omap->units = GetMapserverUnitUsingProj(&(omap->projection));

   }


   omap->extent.minx = map->extent[0];
   omap->extent.miny = map->extent[1];
   omap->extent.maxx = map->extent[2];
   omap->extent.maxy = map->extent[3];
   msMapSetSize(omap, map->width, map->height);

   imageObj *image = msDrawMap(omap, MS_FALSE);
   rasterBufferObj rb;

   image->format->vtable->getRasterBufferHandle(image,&rb);

   map->image = mapcache_image_create(ctx);
   map->image->w = map->width;
   map->image->h = map->height;
   map->image->stride = 4 * map->width;
   map->image->data = malloc(map->width*map->height*4);
   memcpy(map->image->data,rb.data.rgba.pixels,map->width*map->height*4);
   apr_pool_cleanup_register(ctx->pool, map->image->data,(void*)free, apr_pool_cleanup_null);
   msFreeImage(image);
   msFreeMap(omap);
    
    /*
    apr_table_t *params = apr_table_clone(ctx->pool,mapserver->mapserver_default_params);
    apr_table_setn(params,"BBOX",apr_psprintf(ctx->pool,"%f,%f,%f,%f",
             map->extent[0],map->extent[1],map->extent[2],map->extent[3]));
    apr_table_setn(params,"WIDTH",apr_psprintf(ctx->pool,"%d",map->width));
    apr_table_setn(params,"HEIGHT",apr_psprintf(ctx->pool,"%d",map->height));
    apr_table_setn(params,"FORMAT","image/png");
    apr_table_setn(params,"SRS",map->grid_link->grid->srs);
 
    apr_table_overlap(params,mapserver->getmap_params,0);
    if(map->dimensions && !apr_is_empty_table(map->dimensions)) {
       const apr_array_header_t *elts = apr_table_elts(map->dimensions);
       int i;
       for(i=0;i<elts->nelts;i++) {
          apr_table_entry_t entry = APR_ARRAY_IDX(elts,i,apr_table_entry_t);
          apr_table_setn(params,entry.key,entry.val);
       }
 
    }      
    map->data = mapcache_buffer_create(30000,ctx->pool);
    mapcache_http_do_request_with_params(ctx,mapserver->http,params,map->data,NULL,NULL);
    GC_CHECK_ERROR(ctx);
 
    if(!mapcache_imageio_is_valid_format(ctx,map->data)) {
       char *returned_data = apr_pstrndup(ctx->pool,(char*)map->data->buf,map->data->size);
       ctx->set_error(ctx, 502, "mapserver request for tileset %s returned an unsupported format:\n%s",
             map->tileset->name, returned_data);
    }
    */
}

void _mapcache_source_mapserver_query(mapcache_context *ctx, mapcache_feature_info *fi) {
   ctx->set_error(ctx,500,"mapserver source does not support queries");
}

/**
 * \private \memberof mapcache_source_mapserver
 * \sa mapcache_source::configuration_parse()
 */
void _mapcache_source_mapserver_configuration_parse_xml(mapcache_context *ctx, ezxml_t node, mapcache_source *source) {
   ezxml_t cur_node;
   mapcache_source_mapserver *src = (mapcache_source_mapserver*)source;
   if ((cur_node = ezxml_child(node,"mapfile")) != NULL) {
      src->mapfile = apr_pstrdup(ctx->pool,cur_node->txt);
   }
}

/**
 * \private \memberof mapcache_source_mapserver
 * \sa mapcache_source::configuration_check()
 */
void _mapcache_source_mapserver_configuration_check(mapcache_context *ctx, mapcache_source *source) {
   mapcache_source_mapserver *src = (mapcache_source_mapserver*)source;
   /* check all required parameters are configured */
   if(!src->mapfile) {
      ctx->set_error(ctx, 400, "mapserver source %s has no <mapfile> configured",source->name);
   }
   if(!src->mapfile) {
      ctx->set_error(ctx,400,"mapserver source \"%s\" has no mapfile configured",src->source.name);
      return;
   }
   mapObj *map = msLoadMap(src->mapfile, NULL);
   if(!map) {
      ctx->set_error(ctx,400,"failed to load mapfile \"%s\"",src->mapfile);
      return;
   }
   msFreeMap(map);
   msCleanup();
}

mapcache_source* mapcache_source_mapserver_create(mapcache_context *ctx) {
   mapcache_source_mapserver *source = apr_pcalloc(ctx->pool, sizeof(mapcache_source_mapserver));
   if(!source) {
      ctx->set_error(ctx, 500, "failed to allocate mapserver source");
      return NULL;
   }
   mapcache_source_init(ctx, &(source->source));
   source->source.type = MAPCACHE_SOURCE_MAPSERVER;
   source->source.render_map = _mapcache_source_mapserver_render_map;
   source->source.configuration_check = _mapcache_source_mapserver_configuration_check;
   source->source.configuration_parse_xml = _mapcache_source_mapserver_configuration_parse_xml;
   source->source.query_info = _mapcache_source_mapserver_query;
   return (mapcache_source*)source;
}

#endif

/* vim: ai ts=3 sts=3 et sw=3
*/
