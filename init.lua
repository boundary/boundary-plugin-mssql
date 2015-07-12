-- Copyright 2015 Boundary, Inc.
--
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
--    http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.

local framework = require('framework')
local CommandOutputDataSource = framework.CommandOutputDataSource
local DataSource = framework.DataSource
local Plugin = framework.Plugin
local CachedDataSource = framework.CachedDataSource
local os = require('os')
local gsplit = framework.string.gsplit
local pack = framework.util.pack

local params = framework.params

local cmd = {
  path = 'powershell',
  args = {'-NoLogo -NonInteractive -NoProfile -Command Set-ExecutionPolicy UnRestricted; .\\get_metrics.ps1'},
  use_popen = true
}

local ds = CommandOutputDataSource:new(cmd)
local plugin = Plugin:new(params, ds)

function plugin:onParseValues(data)
  local result = {}
  local output = data.output
  if output then
    p(output)
    for v in gsplit(output, '\r\n') do
      local metric, value, source = string.match(v, '([%u_]+)%s([%d.?]+)%s(.+)')
      if metric and value and source then
        source = self.source .. '.' .. source
        table.insert(result, pack(metric, value, nil, source))
      end
    end
  end
  return result
end

plugin:run()
