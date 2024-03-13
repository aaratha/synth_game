extends GraphNode

var v
var range

# Called when the node enters the scene tree for the first time.
func _ready():
	v = 0
	range = [0,1]


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	pass



func _on_close_request():
	queue_free()
