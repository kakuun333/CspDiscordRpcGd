extends Control


# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	CspDiscordRpcGdCppFunctionLibrary.configure_main_window()
	add_child(CspDiscordRpcGdCppFunctionLibrary.build_main_control())


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass
