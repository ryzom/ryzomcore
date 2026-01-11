-- A reference id : object like a string but for reference ids


r2.RefIdMetatable =
{
	__eq = function(op1, op2) return op1.Value == op2.Value end,
	__le = function(op1, op2) return op1.Value <= op2.Value end,
	__lt = function(op1, op2) return op1.Value  < op2.Value end,
	__tostring = function(op) return op.Value end
}



function r2.RefId(value)
	assert(value ~= r2) -- in case of r2:RefId(value)
	if value == nil then value = "" end
	local refId = { Value = tostring(value) }
	setmetatable(refId, r2.RefIdMetatable)
   return refId
end

function r2.isRefId(value)
	return type(value) == "table" and getmetatable(value) == r2.RefIdMetatable
end
