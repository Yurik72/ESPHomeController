import React from "react";

class Button extends React.Component {
    render() {
        const { nostyle } = this.props;
        var dfltclassNames = "btn waves-effect waves-light btn_mode_static ";
        
        const { label,onClick } = this.props;
        const { className } = this.props;
       
        
        return (
          
               <a className={(nostyle ?"":dfltclassNames) +" "+ className}
                    onClick={(ev) => { ev.preventDefault(); onClick(ev) }}
                        href="#"
                    name="action" >
               
                {this.props.children ? this.props.children :label}
                </a>
           
            );
    }
}
export default Button;