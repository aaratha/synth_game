extends Control

var sequencer = load("res://scenes/Sequencer.tscn")
var init_pos = Vector2(40,40)
var node_index = 0

func _on_button_pressed():
	var node = sequencer.instantiate()
	node.position += init_pos + (node_index * Vector2(20,20))
	$GraphEdit.add_child(node)
	node_index += 1
