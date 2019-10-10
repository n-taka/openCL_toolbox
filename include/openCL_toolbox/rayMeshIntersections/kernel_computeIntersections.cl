OCLSTRINGIFY(
    bool rayBoxIntersection(
        const float4 orig,
        const float4 dir,
        const float4 bb_min,
        const float4 bb_max) {
        // This should be precomputed and provided as input
        float4 inv_dir = (float4)(1.0f / dir.x, 1.0f / dir.y, 1.0f / dir.z, 1.0f);
        bool signX, signY, signZ;
        signX = (inv_dir.x < 0.0f);
        signY = (inv_dir.y < 0.0f);
        signZ = (inv_dir.z < 0.0f);
        // http://people.csail.mit.edu/amy/papers/box-jgt.pdf
        // "An Efficient and Robust Ray–Box Intersection Algorithm"
        float tmin, tmax, tymin, tymax, tzmin, tzmax;

        tmin = (((signX) ? bb_max : bb_min).x - orig.x) * inv_dir.x;
        tmax = (((signX) ? bb_min : bb_max).x - orig.x) * inv_dir.x;
        tymin = (((signY) ? bb_max : bb_min).y - orig.y) * inv_dir.y;
        tymax = (((signY) ? bb_min : bb_max).y - orig.y) * inv_dir.y;
        if ((tmin > tymax) || (tymin > tmax))
        {
            return false;
        }
        if (tymin > tmin)
        {
            tmin = tymin;
        }
        if (tymax < tmax)
        {
            tmax = tymax;
        }
        tzmin = (((signZ) ? bb_max : bb_min).z - orig.z) * inv_dir.z;
        tzmax = (((signZ) ? bb_min : bb_max).z - orig.z) * inv_dir.z;
        if ((tmin > tzmax) || (tzmin > tmax))
        {
            return false;
        }
        if (tzmin > tmin)
        {
            tmin = tzmin;
        }
        if (tzmax < tmax)
        {
            tmax = tzmax;
        }
        // t1 == inf
        // if(!( (tmin < t1) && (tmax > t0) ))
        if (!(tmax > 0.0f))
        {
            return false;
        }
        return true;
    } void rayTriangleIntersection(const float4 orig,
                                    const float4 dir,
                                    const float4 vert0,
                                    const float4 vert1,
                                    const float4 vert2,
                                    float4* tuv) {
        float4 edge1, edge2, tvec, pvec, qvec;
        float det, inv_det;

        /* find vectors for two edges sharing vert0 */
        edge1 = vert1 - vert0;
        edge2 = vert2 - vert0;

        /* begin calculating determinant - also used to calculate U parameter */
        pvec = cross(dir, edge2);

        /* if determinant is near zero, ray lies in plane of triangle */
        det = dot(edge1, pvec);

        if (det > 0.000001)
        {
            /* calculate distance from vert0 to ray origin */
            tvec = orig - vert0;

            /* calculate U parameter and test bounds */
            tuv->y = dot(tvec, pvec);
            if (tuv->y < 0.0f || tuv->y > det)
            {
                tuv->x = -1.0f;
                return;
            }

            /* prepare to test V parameter */
            qvec = cross(tvec, edge1);

            /* calculate V parameter and test bounds */
            tuv->z = dot(dir, qvec);
            if (tuv->z < 0.0f || tuv->y + tuv->z > det)
            {
                tuv->x = -1.0f;
                return;
            }
        }
        else if (det < -0.000001)
        {
            /* calculate distance from vert0 to ray origin */
            tvec = orig - vert0;

            /* calculate U parameter and test bounds */
            tuv->y = dot(tvec, pvec);
            /*      printf("*u=%f\n",(float)*u); */
            /*      printf("det=%f\n",det); */
            if (tuv->y > 0.0f || tuv->y < det)
            {
                tuv->x = -1.0f;
                return;
            }

            /* prepare to test V parameter */
            qvec = cross(tvec, edge1);

            /* calculate V parameter and test bounds */
            tuv->z = dot(dir, qvec);
            if (tuv->z > 0.0f || tuv->y + tuv->z < det)
            {
                tuv->x = -1.0f;
                return;
            }
        }
        else
        {
            /* ray is parallel to the plane of the triangle */
            tuv->x = -1.0f;
            return;
        }

        inv_det = 1.0f / det;

        /* calculate t, ray intersects triangle */
        tuv->x = dot(edge2, qvec) / det;
        tuv->y *= inv_det;
        tuv->z *= inv_det;

        return;
    }

    void rayMeshIntersections(const float4 orig,
                               const float4 dir,
                               __constant float4 *V,
                               __constant int4 *F,
                               __constant float4 *bb_min,
                               __constant float4 *bb_max,
                               const int maxHit,
                               const int BBCount,
                               __constant int *elements,
                               __global float4* tuvArray,
                               __global int* idArray) {
        int currentIdx = 0;
        // do traverse...
        while (currentIdx < BBCount)
        {
            // check ray-box intersection (currentIdx)
            if (rayBoxIntersection(orig, dir, bb_min[currentIdx], bb_max[currentIdx]))
            {
                // check this box is leaf or non-leaf
                if (elements[currentIdx] < 0)
                {
                    // non-leaf: traverse
                    currentIdx = currentIdx * 2 + 1;
                }
                else
                {
                    // leaf: ray-triangle intersection
                    const float4 vert0 = V[F[elements[currentIdx]].x];
                    const float4 vert1 = V[F[elements[currentIdx]].y];
                    const float4 vert2 = V[F[elements[currentIdx]].z];
                    float4 tuv;
                    rayTriangleIntersection(orig, dir, vert0, vert1, vert2, &tuv);
                    if (tuv.x > 0.0)
                    {
                        for(int i=0;i<maxHit;++i)
                        {
                            if(tuvArray[i].x == 0 || tuv.x < tuvArray[i].x)
                            {
                                for(int j=maxHit-1;j>i;--j)
                                {
                                    tuvArray[j] = tuvArray[j-1];
                                    idArray[j] = idArray[j-1];
                                }
                                tuvArray[i] = tuv;
                                idArray[i] = elements[currentIdx];
                                break;
                            }
                        }
                    }
                    // then move to
                    // sibling (odd nodes)
                    // sibling of the parent, grand parent, ... (even nodes)
                    while (currentIdx % 2 == 0 && currentIdx != 0)
                    {
                        currentIdx = (currentIdx - 1) / 2;
                    }
                    if (currentIdx == 0)
                    {
                        break;
                    }
                    currentIdx = currentIdx + 1;
                }
            }
            else
            {
                // then move to
                // sibling (odd nodes)
                // sibling of the parent, grand parent, ... (even nodes)
                while (currentIdx % 2 == 0 && currentIdx != 0)
                {
                    currentIdx = (currentIdx - 1) / 2;
                }
                if (currentIdx == 0)
                {
                    break;
                }
                currentIdx += 1;
            }
        }
    } __kernel void computeIntersection_partial(__constant float4 *V,
                                                __constant int4 *F,
                                                __constant float4 *R,
                                                __constant float4 *D,
                                                __constant float4 *bb_min,
                                                __constant float4 *bb_max,
                                                __constant int *elements,
                                                const int maxHit,
                                                const int offset,
                                                const int BBCount,
                                                __global float4 *TUV_partial,
                                                __global int *Id_partial) {
        int rayIdx = get_global_id(0);
        float4 source = R[rayIdx + offset];
        float4 dir = D[rayIdx + offset];

        // do computation
        // distance 0 will never happen
        rayMeshIntersections(source, dir, V, F, bb_min, bb_max, maxHit, BBCount, elements, TUV_partial + rayIdx*maxHit, Id_partial + rayIdx*maxHit);
    } )
