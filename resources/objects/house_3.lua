statics.house_3 =
function (position, rotation)
    return StaticObject(
        "House_3",
        {"outlines", "standard"},
        position,
        15.388/2,
        rotation,
        {
            BoxShape(vec3(position:x(),position:y()+5.01659,position:z()), vec4(0,0,0,1), 0, vec3(15.388/2, 10.2203/2, 15.388/2)),
        },
        {
            [1] = levelResourcePath.."House_3-Lightmap.png"
        }
    )
end
