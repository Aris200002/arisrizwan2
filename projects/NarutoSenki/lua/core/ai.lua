--
-- Parser konfigurasi AI (lihat lua/ai_config.lua)
-- Binding C++: Classes/Systems/LuaBindings.hpp (MonAI, HeroAI, BulletAI, TrapAI, Transform, Battle)
--
local M = {}

local function trim(s) return (s:gsub('^%s+', ''):gsub('%s+$', '')) end

-- Tentukan jenis section dari judul "#...."
local function sectionKind(title)
    title = title:lower()
    if title:find('main') then return 'main' end
    if title:find('trap') then return 'trap' end
    if title:find('trans') then return 'transform' end
    if title:find('mon') then return 'mon' end
    if title:find('bullet') or title:find('proj') then return 'bullet' end
    if title:find('char') or title:find('hero') then return 'hero' end
    return nil
end

function M.load(text)
    local section = nil
    local main = {}

    for rawLine in text:gmatch('[^\r\n]+') do
        local line = trim(rawLine)
        if line ~= '' then
            if line:sub(1, 1) == '#' then
                section = sectionKind(line:sub(2))
                if not section then
                    log('[ai_config] section tidak dikenal: ' .. line)
                end
            elseif section == 'main' then
                for name in line:gmatch('[\'"]?([%w_]+)[\'"]?') do
                    main[#main + 1] = name
                end
            elseif section == 'hero' or section == 'mon' or section == 'bullet' or section == 'transform' or section == 'trap' then
                -- "Nama = AI" atau (khusus Mons) "Nama = AI, Master, MonSambungan"
                -- ("-" berarti kosong)
                local name, rest = line:match('^([%w_]+)%s*=%s*(.+)$')
                local ai, owner, link
                if name then
                    local parts = {}
                    for part in rest:gmatch('[^,]+') do
                        part = trim(part):gsub('^[\'"]', ''):gsub('[\'"]$', '')
                        parts[#parts + 1] = part
                    end
                    ai = parts[1]
                    owner = parts[2]
                    link = parts[3]
                    if owner == '-' then owner = nil end
                    if link == '-' then link = nil end
                    if not ai or not ai:match('^[%w_]+$') then name = nil end
                end
                if name then
                    if section == 'hero' then
                        HeroAI.clone(name, ai)
                    elseif section == 'mon' then
                        MonAI.clone(name, ai, owner, link)
                    elseif section == 'trap' then
                        TrapAI.clone(name, ai)
                    elseif section == 'transform' then
                        Transform.clone(name, ai)
                    else
                        BulletAI.clone(name, ai)
                    end
                else
                    log('[ai_config] baris tidak valid: ' .. line)
                end
            end
        end
    end

    -- Main characters: menggantikan daftar karakter battle bawaan
    if #main > 0 then
        for _, name in ipairs(Battle.chars()) do Battle.removeChar(name) end
        Battle.addChars(main)
        -- jaga-jaga: jangan sampai pool kosong
        if #Battle.chars() == 0 then Battle.resetChars() end
    end
end

return M
