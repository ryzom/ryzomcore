------------------------------
-- 'BaseClass' private code --
------------------------------

---------------------------------------------------------------------------------------------------------
-- copy default implementation

	-- privates, used by baseClass.copy :	
	local srcToCopyMapping = {} -- map each instance in the src tree to its shallow vopy
	
	local function isInstance(inst)
		return (type(inst) == "table" or  type(inst) == "userdata") and inst.InstanceId ~= nil
	end	

   local function isArray(inst)
		return type(inst) == "table" and inst.InstanceId == nil
	end	

	local function isInstanceArray(array)
		return type(array) == "userdata" and array.Size ~= nil
	end	
		

	--------------------------------------------------------------
	-- create canonical copy
	local function canonicalCopy(node)
		if isInstance(node) then
			assert(node.Class ~= nil)			
			local newNode = {}
			for k, v in specPairs(node) do
			-- special case for refId in class definition				
            --inspect(node:getClass())            
				local prop = r2.Classes[node.Class].NameToProp[k]
				if prop and prop.Type == "RefId" then
					newNode[k] = r2.RefId(v)				
				else
					newNode[k] = canonicalCopy(v)
				end
			end						
			return newNode
		elseif isInstanceArray(node) then
			local newArray = {}						
			for k, v in specPairs(node) do
				newArray[k] = canonicalCopy(v)
			end			
			return newArray
		else
			return node
		end
	end

	--------------------------------------------------------------
	-- clone an object into clipboard, refid are unmodified
	local function cloneNode(node, mapping)
		if isInstance(node) then
			assert(node.Class ~= nil)			
			local newNode = r2.newComponent(node.Class)			
			for k, v in pairs(node) do
			-- special case for refId in class definition				
            --inspect(node:getClass())            
            local prop = r2.Classes[node.Class].NameToProp[k]
				if prop and prop.Type == "RefId" then
					newNode[k] = r2.RefId(v)				
				elseif k ~= "InstanceId" and k ~= "Class" then
					newNode[k] = cloneNode(v, mapping)
				end
			end
			mapping[node.InstanceId] = newNode.InstanceId
			return newNode
		elseif isArray(node) then
			local newArray = {}
			-- workaround here : if there are only numbers in the table keys, then sort and insert
			-- this allow to preserve order when duplicating a road / place
			local plainArray = true
			for k, v in pairs(node) do
				if type(k) ~= "number" then
					plainArray = false
					break
				end
			end
			if plainArray then
				local indices = {}
				for k, v in pairs(node) do
					table.insert(indices, k)					
				end
				table.sort(indices)
				for i = 1, table.getn(indices) do
					table.insert(newArray, cloneNode(node[indices[i]], mapping))
				end
			else
				for k, v in pairs(node) do
					newArray[k] = cloneNode(v, mapping)
				end
			end
			return newArray
		else
			return node
		end
	end

	--------------------------------------------------------------
	-- remap refid for internal refs, external refs are left unmodified
	local function renameRefIds(node, mapping)      
		if isInstance(node) then
			assert(node.Class ~= nil)			
			for k, v in pairs(node) do
				-- special case for refId in class definition				
            local prop = r2.Classes[node.Class].NameToProp[k]
				if prop and prop.Type == "RefId" then
               --debugInfo("***" .. tostring(v))               
					if mapping[tostring(v)] ~= nil then
                  --debugInfo("###")
						node[k] = mapping[tostring(v)] -- remap internal ref
					end						
				else
              renameRefIds(v, mapping)
            end
			end
			mapping[node] = newNode   
		elseif type(node) == "table" then			
			for k, v in specPairs(node) do
				renameRefIds(v, mapping)
			end
			return newArray
		end		
	end

function baseClass.copy(this)	
	return canonicalCopy(this)	
end

function baseClass.newCopy(this)	
	table.clear(srcToCopyMapping)
	local result = cloneNode(this, srcToCopyMapping)   
	renameRefIds(result, srcToCopyMapping)
	-- if the name is a generated one, create a new name
	if result.Name ~= nil and r2:isPostFixedByNumber(ucstring(result.Name)) then
		result.Name = r2:genInstanceName(ucstring(result.Name)):toUtf8()
	end  

	-- the copy of a undeletable object is a deltable object
	if result.Deletable  ~= nil and result.Deletable==0 then
		result.Deletable = 1
	end 
	return result
end
