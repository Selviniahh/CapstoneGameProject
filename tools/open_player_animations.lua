local root = app.params["root"]

if not root or root == "" then
  error("Missing Player directory. Pass it with --script-param root=/path/to/Player")
end

root = app.fs.normalizePath(root)
if not app.fs.isDirectory(root) then
  error("Player directory does not exist: " .. root)
end

local sequences = {}

local function collectSequences(directory)
  local entries = app.fs.listFiles(directory)
  table.sort(entries)

  for _, entry in ipairs(entries) do
    local fullPath = app.fs.joinPath(directory, entry)

    if app.fs.isDirectory(fullPath) then
      -- Textures is the source archive. Opening it would add hundreds of
      -- duplicate documents alongside the organized gameplay animations.
      if entry ~= "Textures" then
        collectSequences(fullPath)
      end
    elseif app.fs.fileExtension(entry):lower() == "png" then
      local title = app.fs.fileTitle(entry)
      local prefix, frameDigits = title:match("^(.-)(%d+)$")

      if prefix and frameDigits then
        local key = app.fs.joinPath(directory, prefix)
        local frameNumber = tonumber(frameDigits)
        local current = sequences[key]

        if not current or frameNumber < current.frameNumber then
          sequences[key] = {
            firstFrame = fullPath,
            frameNumber = frameNumber
          }
        end
      else
        -- Also keep an isolated PNG accessible as a one-frame document.
        sequences[fullPath] = {
          firstFrame = fullPath,
          frameNumber = 0
        }
      end
    end
  end
end

collectSequences(root)

local keys = {}
for key in pairs(sequences) do
  table.insert(keys, key)
end
table.sort(keys)

for _, key in ipairs(keys) do
  app.command.OpenFile {
    ui = false,
    filename = sequences[key].firstFrame,
    sequence = "yes",
    repeat_checkbox = false
  }
end

if #keys == 0 then
  error("No numbered PNG animation sequences found under: " .. root)
end

print("Opened " .. #keys .. " Player animation sequences.")
