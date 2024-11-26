return function(name,super)
    local clazz=
    {
        name=name,
        super=super,
    }

    if super then
        setmetatable(clazz,{__index=super})
    end

    clazz.new=function(...)
        local inst={}
        setmetatable(inst,{__index=clazz})

        if clazz.ctor then
            clazz.ctor(inst,...)
        end

        return inst
    end

    return clazz
end